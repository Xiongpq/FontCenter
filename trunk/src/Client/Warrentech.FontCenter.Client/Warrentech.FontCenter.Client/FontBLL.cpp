#include "stdafx.h"
#include "FontBLL.h"
#include <string>
#include <io.h>
#include <assert.h>
#include "curl/curl.h"
#include "json/json.h"
#include "zip/unzip.h"
#include "zip/zip.h"
#include <tchar.h>
#include <algorithm>
using namespace std;
#pragma comment(lib, "libcurl.lib")

wstring FontBasicPath;
const string WebApiBasicUrl = "http://you url";  //设置成你的URL，我用的是百度BAE
const string SyncCadFontUrl = WebApiBasicUrl + "/sync_cad_font";
const string UploadCadFontUrl = WebApiBasicUrl + "/upload_cad_font";
const string DownloadCadFontUrl = "http://you url";  //设置成你的URL，我用的是百度云存储
const string ReportMissingCadFontUrl = WebApiBasicUrl + "/report_missing_cad_font";

FontBLL::FontBLL()
{
	TCHAR szFilePath[MAX_PATH + 1];
	GetModuleFileName(NULL, szFilePath, MAX_PATH);
	(_tcsrchr(szFilePath, _T('\\')))[1] = 0;
	std::wstring autoCADBasicDir(szFilePath);
	FontBasicPath = autoCADBasicDir + L"Fonts";
	//CString webApiBasicUrl;
	//GetPrivateProfileString(L"WebApi", L"WebApiBasicUrl", L"", webApiBasicUrl.GetBuffer(MAX_PATH), MAX_PATH, L"config.ini");
}

struct MemoryStruct {
	char *memory;
	size_t size;
};

static void *myrealloc(void *ptr, size_t size)
{
	if (ptr)
		return realloc(ptr, size);
	else
		return malloc(size);
}

static size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)data;

	mem->memory = (char *)myrealloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory) {
		memcpy(&(mem->memory[mem->size]), ptr, realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;
	}
	return realsize;
}

wstring UTF8ToWString(const char* lpcszString)
{
	int len = strlen(lpcszString);
	int unicodeLen = ::MultiByteToWideChar(CP_UTF8, 0, lpcszString, -1, NULL, 0);
	wchar_t* pUnicode;
	pUnicode = new wchar_t[unicodeLen + 1];
	memset((void*)pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	::MultiByteToWideChar(CP_UTF8, 0, lpcszString, -1, (LPWSTR)pUnicode, unicodeLen);
	wstring wstrReturn(pUnicode);
	delete[] pUnicode;
	return wstrReturn;
}

string WStringToUTF8(const wchar_t* lpwcszWString)
{
	char* pElementText;
	int iTextLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, NULL, 0, NULL, NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
	::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, pElementText, iTextLen, NULL, NULL);
	std::string strReturn(pElementText);
	delete[] pElementText;
	return strReturn;
}

wstring StringToWString(const string& str)
{
	LPCSTR pszSrc = str.c_str();
	int nLen = MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, NULL, 0);
	if (nLen == 0)
		return std::wstring(L"");

	wchar_t* pwszDst = new wchar_t[nLen];
	if (!pwszDst)
		return std::wstring(L"");

	MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, pwszDst, nLen);
	std::wstring wstr(pwszDst);
	delete[] pwszDst;
	pwszDst = NULL;

	return wstr;
}

string WStringToString(const wstring& wstr)
{
	LPCWSTR pwszSrc = wstr.c_str();
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);
	if (nLen == 0)
		return std::string("");

	char* pszDst = new char[nLen];
	if (!pszDst)
		return std::string("");

	WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
	std::string str(pszDst);
	delete[] pszDst;
	pszDst = NULL;

	return str;
}

vector<wstring> get_all_fileName(const wchar_t* path, const wchar_t* ext)
{
	vector<wstring> result;
	_tfinddata64_t c_file;
	intptr_t hFile;
	wstring root;
	root.append(path);
	root.append(L"\\*");
	root.append(ext);

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

	return result;
}

size_t read_file_data(char *bufptr, size_t size, size_t nitems, FILE *userp)
{
	size_t read;
	read = fread(bufptr, size, nitems, userp);
	return read;
}

size_t write_file_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	int written = fwrite(ptr, size, nmemb, stream);
	return written;
}

bool upload_file(const wstring path)
{
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
}

wstring download_file(wstring fontName)
{
	CURL *curl;
	CURLcode res;
	FILE *fp;
	wstring fullPath = FontBasicPath + L"\\" + fontName + L".zip";
	_wfopen_s(&fp, fullPath.c_str(), L"wb");
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
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		//curl_easy_setopt(curl, CURLOPT_HEADER, 1);
		res = curl_easy_perform(curl);
		if (CURLE_OK == res) {
			fclose(fp);

			char *ct;
			res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
			if ((CURLE_OK == res) && ct) {
				std::string contentType(ct);
				curl_easy_cleanup(curl);
				if (contentType == "application/zip") {
					result = true;
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

	if (result) {
		return fullPath;
	}
	else {
		_wremove(fullPath.c_str());
		return L"";
	}
}

void upload_font(Json::Value root)
{
	const Json::Value uploadFontList = root["upload"];
	if (uploadFontList.size() <= 0) {
		return;
	}

	//每个文件一个压缩包
	for (int i = 0; i < uploadFontList.size(); i++) {
		wstring fileName = UTF8ToWString(uploadFontList[i].asString().c_str());
		wstring zipFilePath = FontBasicPath + L"\\" + fileName + L".zip";
		HZIP hz = CreateZip(zipFilePath.c_str(), 0);
		ZipAdd(hz, fileName.c_str(), (FontBasicPath + L"\\" + fileName).c_str());
		CloseZip(hz);

		upload_file(zipFilePath);
	}

	//合成一个压缩包
	/*wstring zipFilePath = FontBasicPath + L"\\" + L"Fonts.zip";
	HZIP hz = CreateZip(zipFilePath.c_str(), 0);
	for (int i = 0; i < uploadFontList.size(); i++) {
	wstring fileName = UTF8ToWString(uploadFontList[i].asString().c_str());
	ZipAdd(hz, fileName.c_str(), (FontBasicPath + L"\\" + fileName).c_str());
	}
	CloseZip(hz);*/

	//upload_file(WStringToUTF8(zipFilePath.c_str()).c_str());
}

void unzip(wstring &filePath)
{
	HZIP hz = OpenZip(filePath.c_str(), 0);
	ZIPENTRY ze;
	GetZipItem(hz, -1, &ze);
	int numitems = ze.index;
	for (int i = 0; i < numitems; i++) {
		GetZipItem(hz, i, &ze);
		UnzipItem(hz, i, (FontBasicPath + _T("\\") + ze.name).c_str());
	}
	CloseZip(hz);
}

wstring download_fonts(const vector<wstring> needFontList, const vector<wstring> existFontList)
{
	if (needFontList.size() == 0) {
		return L"";
	}

	//bool isPrintWaitInfo = false;
	wstring downloadFontName = L"";
	for (vector<wstring>::size_type i = 0; i != needFontList.size(); ++i) {
		bool isExist = false;
		for (vector<wstring>::size_type x = 0; x != existFontList.size(); ++x) {
			if (_wcsicmp(existFontList[x].c_str(), needFontList[i].c_str()) == 0) {
				isExist = true;
				break;
			}
		}

		if (!isExist) {
			//if (!isPrintWaitInfo) {
			//	acutPrintf(_T("正在下载字体，请稍候...\n"));
			//	isPrintWaitInfo = true;
			//}

			wstring resultStr;
			wstring filePath = download_file(needFontList[i]);
			if (filePath.empty()) {
				//resultStr = _T("未能在云端找到字体：") + needFontList[i] + L"\n";
				//acutPrintf(resultStr.c_str());
				continue;
			}
			unzip(filePath);
			_wremove(filePath.c_str());
			downloadFontName.append(needFontList[i]);
			downloadFontName.append(L",");
			//resultStr = _T("成功下载字体：") + needFontList[i] + L"\n";
			//acutPrintf(resultStr.c_str());
		}
	}
	return downloadFontName;
}

void upload_fonts(const vector<wstring> &existFontList)
{
	CURL *curl;
	struct MemoryStruct chunk;
	chunk.memory = NULL;
	chunk.size = 0;

	curl = curl_easy_init();

	string postData = "fontlist=";
	wstring existFontListStr;
	for (vector<wstring>::size_type x = 0; x != existFontList.size(); ++x) {
		existFontListStr.append(existFontList[x]);
		existFontListStr.append(L",");
	}
	postData += WStringToUTF8(existFontListStr.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
	curl_easy_setopt(curl, CURLOPT_URL, SyncCadFontUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

	CURLcode res = curl_easy_perform(curl);
	if (CURLE_OK == res) {
		curl_easy_cleanup(curl);

		Json::Reader reader;
		Json::Value root;
		if (reader.parse(chunk.memory, root)) {
			upload_font(root);
		}
	}
}

wstring sync_cad_font(vector<wstring> needFontList)
{
	curl_global_init(CURL_GLOBAL_ALL);

	vector<wstring> existFontList = get_all_fileName(FontBasicPath.c_str(), L".*");

	wstring downloadFontName = download_fonts(needFontList, existFontList);

	upload_fonts(existFontList);

	curl_global_cleanup();

	return downloadFontName;
}

void trim_prefix(wstring &str, wstring splitStr)
{
	wstring::size_type extensionIndex = str.find_last_of(splitStr);
	if (extensionIndex != wstring::npos) {
		str.erase(0, extensionIndex + splitStr.size());
	}
}

void AddToFontList(const TCHAR* pFontName, vector<wstring> &fontList)
{
	std::wstring pFontNameStr(pFontName);
	if (pFontNameStr != L"") {
		wstring::size_type extensionIndex = pFontNameStr.find(L".");
		if (extensionIndex == wstring::npos) {
			pFontNameStr.append(L".shx");
		}
		trim_prefix(pFontNameStr, L"\\");

		extensionIndex = pFontNameStr.find_last_of(L"\\");
		if (extensionIndex != wstring::npos) {
			wstring test = pFontNameStr.erase(0, extensionIndex);
		}

		bool isExist = false;
		for (vector<TCHAR>::size_type i = 0; i != fontList.size(); ++i) {
			if (_wcsicmp(fontList[i].c_str(), pFontNameStr.c_str()) == 0) {
				isExist = true;
				break;
			}
		}

		if (!isExist) {
			fontList.push_back(pFontNameStr);
		}
	}
}

vector<wstring> GetFontList()
{
	Acad::ErrorStatus es;
	AcApDocument* pDoc;
	AcDbDatabase* pDb;
	pDoc = acDocManager->curDocument();
	//acDocManager->lockDocument(pDoc);
	pDb = pDoc->database();

	AcDbTextStyleTable* pTextTbl;
	AcDbTextStyleTableIterator *pTextIterator;
	es = pDb->getTextStyleTable(pTextTbl, AcDb::kForRead);
	pTextTbl->newIterator(pTextIterator);

	vector<wstring> fontList;
	for (pTextIterator->start(); !pTextIterator->done(); pTextIterator->step()) {
		AcDbTextStyleTableRecord *pTextRecord;
		es = pTextIterator->getRecord(pTextRecord, AcDb::kForRead);

		TCHAR* pFontName = NULL;
		es = pTextRecord->fileName(pFontName);
		if (es == Acad::eOk) {
			AddToFontList(pFontName, fontList);
		}

		es = pTextRecord->bigFontFileName(pFontName);
		if (es == Acad::eOk) {
			AddToFontList(pFontName, fontList);
		}
	}
	pTextTbl->close();

	return fontList;
}

void FontBLL::sync_font()
{
	vector<wstring> fontList = GetFontList();

	wstring downloadFontName = sync_cad_font(fontList);
	if (downloadFontName != L"") {
		downloadFontName = downloadFontName.substr(0, downloadFontName.size() - 1);
		wstring resultStr = _T("已成功为您下载缺失字体：") + downloadFontName + _T("。\n请重新打开AutoCAD！");
		acedAlert(resultStr.c_str());
	}
	else {
		//acedAlert("");
	}
}

unsigned __stdcall FontBLL::run(void * pThis)
{
	FontBLL * pthX = (FontBLL*)pThis;   // the tricky cast  
	pthX->sync_font();           // now call the true entry-point-function  
	return 1;
}

FontBLL::~FontBLL()
{
}
