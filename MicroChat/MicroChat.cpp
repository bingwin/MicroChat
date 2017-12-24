// MicroChat.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "MicroChat.h"
#include "LoginWnd.h"
#include "MainWnd.h"
#include "interface.h"
#include "friendWnd.h"



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	CPaintManagerUI::SetInstance(hInstance);

	//载入Loading界面 3s后自动销毁
	/*CLoadingWnd *pLoading = new CLoadingWnd();
	pLoading->Create(NULL, _T("MicroChat"), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
	pLoading->ShowModal();
	delete pLoading;
	pLoading = NULL;*/

	//载入登录界面
	pLoginWnd->Create(NULL, _T("MicroChat"), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
	pLoginWnd->ShowModal();
	delete pLoginWnd;

	//卸载SDK
	StopSDK();

    return 0;
}



