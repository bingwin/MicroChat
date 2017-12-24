#include "stdafx.h"
#include "callback.h"
#include "interface.h"
#include "../fun.h"
#include "../MainWnd.h"
#include "../LoginWnd.h"
#include <shellapi.h>
#include "../friendWnd.h"

#define hLoginWnd	pLoginWnd->GetHWND()	//登录窗口句柄
#define hMainWnd	pMainWnd->GetHWND()		//wx主窗口句柄


//当前登录账号wxid
char g_wxid[100] = { 0 };

//当前登录账号uin
DWORD g_dwUin = 0;

static void MessageBoxThread(LPCTSTR szMsg)
{
	MessageBox(hLoginWnd, szMsg, NULL, NULL);
}
static void ForceQuit(CString str)
{
	CloseHandle(CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)MessageBoxThread, str.GetBuffer(), NULL, NULL));
	Sleep(3 * 1000);
	TerminateProcess(GetCurrentProcess(), 0);
}
#define FORCE_QUIT(szMsg) ForceQuit(szMsg)

//回调失败处理
void HandleCallBackError(void *result, int nCgiType, int nTaskId, int nCode)
{
	CString strErrMsg;

	switch (nCode)
	{
	case CGI_CODE_LOGIN_NEED_SCAN_QRCODE:
		//扫码登录流程
		PostMessage(hLoginWnd, WM_LOGIN_SCAN_QRCODE, (WPARAM)result, (LPARAM)nCode);
		break;
	case CGI_CODE_LOGIN_NEED_MOBILE_MSG:
		//发送接收短信验证码请求
		RecvMobileVerifyCode();
		break;
	case CGI_CODE_LOGIN_FAIL:
		//其他登录错误
		PostMessage(hLoginWnd, WM_SHOW_LOGIN_RESULT, (WPARAM)result, (LPARAM)nCode);
		break;
	case CGI_CODE_NETWORK_ERR:
		//网络异常,弹框退出
		strErrMsg.Format(L"网络异常,即将退出......\ncgi type:%d,taskid:%d,code:%d", nCgiType, nTaskId, nCode);
		FORCE_QUIT(strErrMsg);
		break;
	case CGI_CODE_DECRYPT_ERR:
	case CGI_CODE_PARSE_PROTOBUF_ERR:
	case CGI_CODE_UNPACK_ERR:
		//通讯协议出错,弹框退出
		strErrMsg.Format(L"通信协议出错,即将退出......\ncgi type:%d,taskid:%d,code:%d", nCgiType, nTaskId, nCode);
		FORCE_QUIT(strErrMsg);
		break;
	case CGI_CODE_LOGIN_ECDH_ERR:
		//ECDH协商失败,弹框退出
		strErrMsg.Format(L"ECDH握手失败,即将退出......\ncgi type:%d,taskid:%d,code:%d", nCgiType, nTaskId, nCode);
		FORCE_QUIT(strErrMsg);
		break;
	case CGI_CODE_LOGIN_SCAN_QRCODE_ERR:
		//扫码授权url获取失败,弹框退出
		strErrMsg.Format(L"扫码授权地址获取失败,即将退出......\ncgi type:%d,taskid:%d,code:%d", nCgiType, nTaskId, nCode);
		FORCE_QUIT(strErrMsg);
		break;
	case CGI_ERR_UNKNOWN:
		//未知错误,弹框退出
		strErrMsg.Format(L"发生未知异常,即将退出......\ncgi type:%d,taskid:%d,code:%d", nCgiType, nTaskId, nCode);
		FORCE_QUIT(strErrMsg);
		break;
	default:
		break;
	}
}

void SDKCallBack(void *result, int nCgiType, int nTaskId, int nCode)
{
	if (nCode)
	{
		//失败处理(result内存在UI线程中释放)
		HandleCallBackError(result, nCgiType,nTaskId,nCode);
	}
	else
	{
		switch (nCgiType)
		{
		case CGI_TYPE_MANUALAUTH:
			//登陆成功,显示主窗口
			LoginCallBack((LoginResult *)result);			
			break;
		case CGI_TYPE_BIND:
			//短信验证
			MobileVerifyCallBack((MobileVerifyResult *)result);
			break;
		case CGI_TYPE_NEWINIT:
			NewInitCallBack((NewInitResult *)result);
			break;
		case CGI_TYPE_NEWSYNC:
			//同步成功，刷新UI
			NewSyncCallBack((NewSyncResult *)result);
			break;
		case CGI_TYPE_SEARCHCONTACT:
			//搜索联系人
			SearchContactCallBack((SearchContactResult *)result);
			break;
		case CGI_TYPE_VERIFYUSER:
			//加好友
			AddNewFriendCallBack((VerifyUserResult *)result);
			break;
		default:
			break;
		}
	}	
}

void LoginCallBack(LoginResult *res)
{
	//登陆成功,保存wxid和uin
	g_dwUin = res->dwUin;
	strcpy_s(g_wxid,res->szWxid);
	
	if (res)
	{
		SAFE_DELETE(res);
	}

	/** @brief 登录成功 **/
	PostMessage(hLoginWnd,WM_LOGIN_SUCC, NULL, NULL);
}

void MobileVerifyCallBack(MobileVerifyResult *res)
{
	PostMessage(hLoginWnd, WM_MOBILE_VERIFY, (WPARAM)res, NULL);
}

void NewInitCallBack(NewInitResult *res)
{
	if (!res)	return;

	if (res->dwContanct)
	{
		for (int i = 0; i < res->dwContanct; i++)
		{
			PostMessage(hMainWnd, WM_MAIN_ADD_FRIEND, (WPARAM)res->ppContanct[i], NULL);
		}
		for (int i = res->dwContanct; i < res->dwSize; i++)
		{
			SAFE_DELETE(res->ppContanct[i]);
		}
	}
	if (res->dwNewMsg)
	{
		for (int i = 0; i < res->dwNewMsg; i++)
		{
			//过滤weixin说的废话
			if ((res->ppNewMsg[i]->nType == 10002 || res->ppNewMsg[i]->nType == 9999) && !strcmp(res->ppNewMsg[i]->szFrom, "weixin"))
			{
				SAFE_DELETE(res->ppNewMsg[i]);
				continue;
			}
			PostMessage(hMainWnd, WM_MAIN_ADD_NEW_MSG, (WPARAM)res->ppNewMsg[i], NULL);
		}
		for (int i = res->dwNewMsg; i < res->dwSize; i++)
		{
			SAFE_DELETE(res->ppNewMsg[i]);
		}
	}
	
	//初始化结束后要主动请求一次同步
	NewSync();
}

void NewSyncCallBack(NewSyncResult *res)
{
	if (!res)	return;

	//处理好友消息
	if (res->dwContanct)
	{
		for (int i = 0; i < res->dwContanct; i++)
		{
			PostMessage(hMainWnd, WM_MAIN_ADD_FRIEND, (WPARAM)res->ppContanct[i], NULL);
		}
		for (int i = res->dwContanct; i < res->dwSize; i++)
		{
			SAFE_DELETE(res->ppContanct[i]);
		}
	}

	if (res->dwNewMsg)
	{
		for (int i = 0; i < res->dwNewMsg; i++)
		{
			//过滤weixin说的废话
			if ((res->ppNewMsg[i]->nType == 10002 || res->ppNewMsg[i]->nType == 9999) && !strcmp(res->ppNewMsg[i]->szFrom,"weixin") )
			{
				SAFE_DELETE(res->ppNewMsg[i]);
				continue;
			}
			PostMessage(hMainWnd, WM_MAIN_ADD_NEW_MSG, (WPARAM)res->ppNewMsg[i], NULL);
		}
		for (int i=res->dwNewMsg;i<res->dwSize;i++)
		{
			SAFE_DELETE(res->ppNewMsg[i]);
		}
	}

}

void SearchContactCallBack(SearchContactResult *res)
{
	PostMessage(pFriendWnd->GetHWND(), WM_FRIEND_SEARCH_CONTACT, (WPARAM)res, NULL);
}

void AddNewFriendCallBack(VerifyUserResult *res)
{
	PostMessage(pFriendWnd->GetHWND(), WM_FRIEND_ADD_NEW_FRIEND, (WPARAM)res, NULL);
}

