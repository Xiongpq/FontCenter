FontCenter
==========

# 概述： #
使用AutoCAD的过程中，我们常常因为缺失字体而烦恼，本插件就是为了解决这个问题。

插件采用WEB服务器 + CAD插件方式。WEB服务器使用Python编写，部署在百度BAE上；CAD插件使用C++开发，在AutoCAD中使用命令“APPLOAD”加载该插件。

在CAD中打开新的DWG文档后，插件会自动比较DWG文档所需字体以及CAD的Font目录下的字体，如果有缺失字体，则自动到WEB服务器下载；如果有服务器上没有的字体，就悄悄上传到服务器。

# 下载： #
## 源代码： ##
https://github.com/Xiongpq/FontCenter

编译源代码需要ObjectARX，请自行下载。

## 客户端： ##
http://pan.baidu.com/s/1pJPk6mR

下载客户端后，可以在AutoCAD中，输入“APPLOAD”命令加载相应的Warrentech.FontCenter.Client.arx文件即可加载该插件。请注意AutoCAD不同版本应该加载不同的DLL，例如：AutoCAD 2008 32位，应加载2008_X86文件夹下的DLL。

# 主要代码： #
## 一、在LoadDwg时添加处理字体代码 ##
客户端使用C++编写，调用AutoCAD的ObjectARX C++ API，在AutoCAD的On_kLoadDwgMsg事件中使用多线程做字体的下载及上传，下面是主要的代码：

    virtual AcRx::AppRetCode On_kLoadDwgMsg(void *pkt){
        AcRx::AppRetCode retCode = AcRxArxApp::On_kLoadDwgMsg(pkt);

        try {
            acutPrintf(_T("正在检测该文件字体设置，若有缺失将自动下载...\n"));

            HANDLE   hth1;
            unsigned  threadID;
            FontBLL *fontBLL = new FontBLL();
            hth1 = (HANDLE)_beginthreadex(NULL, 0, FontBLL::run, fontBLL, CREATE_SUSPENDED, &threadID);

            if (hth1 != 0) {
                ResumeThread(hth1);
            }
        }
        catch (...) {}

        return (retCode);
    }
## 二、获取当前DWG文档需要的字体名称 ##
使用ObjectARX接口获取AcDbTextStyleTableIterator，循环这个迭代器，将字体文档的字体名称、大字体名称都加入一个vector<wstring>中，以便后面和Font文件夹中的字体名称进行比较。

注意要对字体名称做一些处理，以及重复性检查，详见源代码。

    Acad::ErrorStatus es;
    AcApDocument* pDoc;
    AcDbDatabase* pDb;
    pDoc = acDocManager->curDocument();
    pDb = pDoc->database();

    AcDbTextStyleTable* pTextTbl;
    AcDbTextStyleTableIterator *pTextIterator;
    es = pDb->getTextStyleTable(pTextTbl, AcDb::kForRead);
    pTextTbl->newIterator(pTextIterator);        //获取迭代器

    vector<wstring> fontList;
    for (pTextIterator->start(); !pTextIterator->done(); pTextIterator->step()) {
        AcDbTextStyleTableRecord *pTextRecord;
        es = pTextIterator->getRecord(pTextRecord, AcDb::kForRead);

        TCHAR* pFontName = NULL;
        es = pTextRecord->fileName(pFontName);   //获取字体名称
        if (es == Acad::eOk) {
            AddToFontList(pFontName, fontList);
        }

        es = pTextRecord->bigFontFileName(pFontName);   //获取大字体名称
        if (es == Acad::eOk) {
            AddToFontList(pFontName, fontList);
        }
    }
    pTextTbl->close();
## 三、获取本地字体列表 ##
获取本地AutoCAD安装目录下Font文件夹下已存在的字体列表。

    vector<wstring> result;
    _tfinddata64_t c_file;
    intptr_t hFile;
    wstring root;
    root.append(path);        //路径
    root.append(L"\\*");
    root.append(ext);         //扩展名

    hFile = _tfindfirst64(root.c_str(), &c_file);
    if (hFile == -1)
        return result;

    do {
        if (_tcslen(c_file.name) == 1 && c_file.name[0] == _T('.')
            || _tcslen(c_file.name) == 2 && c_file.name[0] == _T('.') && c_file.name[1] == _T('.'))
            continue;

        result.push_back(c_file.name);

    } while (_tfindnext64(hFile, &c_file) == 0);
    _findclose(hFile);
## 四、下载缺失字体 ##
插件使用curl类库做HTTP请求，到WEB服务器下载需要的文件。服务器上的字体文件全都是ZIP文件，所以下载下来后，需要解压。另外，如果没有下载到需要的字体，则会通过WEB服务器的report_missing_cad_font这个API，报告服务器。

    CURL *curl;
    CURLcode res;
    FILE *fp;
    wstring fullPath = FontBasicPath + L"\\" + fontName + L".zip";
    _wfopen_s(&fp, fullPath.c_str(), L"wb");   //创建ZIP文件
    if (fp == NULL) {
        return false;
    }
    transform(fontName.begin(), fontName.end(), fontName.begin(), towlower);
    bool result = false;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, (DownloadCadFontUrl + "/" + WStringToUTF8(fontName.c_str()) + ".zip").c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);      //请求服务器
        if (CURLE_OK == res) {
            fclose(fp);

            char *ct;
            res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
            if ((CURLE_OK == res) && ct) {
                std::string contentType(ct);
                curl_easy_cleanup(curl);
                if (contentType == "application/zip") {    //判断下载文件的类型
                    result = true;                           //如果不是"application/zip"类型，就说明服务器上也没有这个字体
                }
                else {
                    CURL *reportCurl;
                    reportCurl = curl_easy_init();
                    curl_easy_setopt(reportCurl, CURLOPT_URL, ReportMissingCadFontUrl.c_str());
                    string postData = "keyword=" + WStringToUTF8(fontName.c_str());
                    curl_easy_setopt(reportCurl, CURLOPT_POSTFIELDS, postData.c_str());
                    res = curl_easy_perform(reportCurl);
                    curl_easy_cleanup(reportCurl);
                }
            }
        }
    }
## 五、上传客户端字体到服务器 ##
服务器端虽然预置了2000多种AutoCAD字体，但仍然不是最全面的，所以如果客户端存在服务器端没有的字体，插件就会上传该字体到服务器。

首先，将本地字体名称列表POST到服务器，服务器会比较服务器上的字体列表，并将需要上传的字体列表通过JSON数据返回。

然后，客户端收到字体列表后，将需要上传的字体压缩打包，通过WEB API上传到服务器端。

    CURL *curl;
    CURLcode res;
    struct curl_httppost *formpost = NULL;
    struct curl_httppost *lastptr = NULL;

    curl_formadd(&formpost,
                 &lastptr,
                 CURLFORM_PTRNAME, "file",
                 CURLFORM_FILE, WStringToString(path).c_str(),
                 CURLFORM_END);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, UploadCadFontUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        _wremove(path.c_str());
        return true;
    }

    _wremove(path.c_str());
    return false;
## 捐赠： ##
如果您觉得这个项目对您有帮助，非常感谢你的捐赠

[![捐赠WeChat.NET](https://img.alipay.com/sys/personalprod/style/mc/btn-index.png)](https://me.alipay.com/xiongpq)