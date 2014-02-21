// (C) Copyright 2002-2007 by Autodesk, Inc. 
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted, 
// provided that the above copyright notice appears in all copies and 
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting 
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM "AS IS" AND WITH ALL FAULTS. 
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC. 
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to 
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
//

//-----------------------------------------------------------------------------
//----- acrxEntryPoint.cpp
//-----------------------------------------------------------------------------
#include "StdAfx.h"
#include "resource.h"
#include "acdocman.h"
#include "FontBLL.h"
#include <vector>
#include <string>
#include <process.h>
#include <tchar.h>
using std::vector;
using namespace std;

//-----------------------------------------------------------------------------
#define szRDS _RXST("")

//void print_hello()
//{
//	acutPrintf(L"嗨......\n");
//}

//-----------------------------------------------------------------------------
//----- ObjectARX EntryPoint
class CWarrentechFontCenterClientApp : public AcRxArxApp {

public:
	CWarrentechFontCenterClientApp() : AcRxArxApp() {}

	virtual AcRx::AppRetCode On_kInitAppMsg(void *pkt) {
		// TODO: Load dependencies here

		// You *must* call On_kInitAppMsg here
		AcRx::AppRetCode retCode = AcRxArxApp::On_kInitAppMsg(pkt);

		// TODO: Add your initialization code here
		//acutPrintf(L"On_kInitAppMsg\n");
		//acedRegCmds->addCommand(_T("FontCenter"), _T("Hello"), _T("Hello"), ACRX_CMD_MODAL, print_hello);

		return (retCode);
	}

	virtual AcRx::AppRetCode On_kUnloadAppMsg(void *pkt) {
		// TODO: Add your code here
		//acutPrintf(L"On_kUnloadAppMsg\n");
		//acedRegCmds->removeGroup(_T("FontCenter"));

		// You *must* call On_kUnloadAppMsg here
		AcRx::AppRetCode retCode = AcRxArxApp::On_kUnloadAppMsg(pkt);

		// TODO: Unload dependencies here	

		return (retCode);
	}

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
		/*catch (CMemoryException* e) {
		}
		catch (CFileException* e) {
		}
		catch (CException* e) {
		}*/
		catch (...) {}

		return (retCode);
	}

	virtual AcRx::AppRetCode On_kUnLoadDwgMsg(void *pkt){
		//acutPrintf(L"On_kUnLoadDwgMsg\n");

		AcRx::AppRetCode retCode = AcRxArxApp::On_kUnloadDwgMsg(pkt);

		return (retCode);
	}

	virtual void RegisterServerComponents() {
	}
};

//-----------------------------------------------------------------------------
IMPLEMENT_ARX_ENTRYPOINT(CWarrentechFontCenterClientApp)

