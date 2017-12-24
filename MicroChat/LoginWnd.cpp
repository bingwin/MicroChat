
#include "StdAfx.h"
#include "Resource.h"
#include "LoginWnd.h"
#include "MainWnd.h"
#include "interface.h"
#include "Business/callback.h"
#include <shellapi.h>

typedef int (WINAPI *MessageBoxTimeoutW)(IN HWND hWnd, IN LPCWSTR lpText, IN LPCWSTR lpCaption, IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);
static MessageBoxTimeoutW MessageBoxTimeout = (MessageBoxTimeoutW)GetProcAddress(GetModuleHandle(L"user32.dll"), "MessageBoxTimeoutW");


CLoginWnd *CLoginWnd::m_Instance = NULL;

void CLoginWnd::QuitWithMsg(CString strMsg, DWORD dwTimeout /*= 3 * 1000*/)
{
	if (MessageBoxTimeout)
	{
		MessageBoxTimeout(GetHWND(), strMsg, L"Error", NULL, 0, dwTimeout);
	}
	else
	{
		MessageBox(GetHWND(), strMsg, L"Error", NULL);
	}

	TerminateProcess(GetCurrentProcess(), 0);
}

LRESULT CLoginWnd::ScanQrcode(UINT uMsg, LoginResult *res, int error_code, BOOL& bHandled)
{
	if (!res)	QuitWithMsg(L"登录异常,即将退出!");

	//显示提示消息
	if (!res->szMsg || !strlen(res->szMsg))
	{
		strcpy(res->szMsg, "未知错误");
	}
	ShowLoginResult("错误码:%d,失败原因:%s", error_code, res->szMsg);

	//弹出页面
	ShellExecute(NULL, L"open", L"iexplore.exe", CA2W(res->szUrl), NULL, SW_SHOWNORMAL);

	//卡住主界面,等待用户扫码成功后重新登录
	MessageBox(GetHWND(), L"扫码授权成功后请重新登录", L"扫码授权", NULL);

	SAFE_DELETE(res);

	bHandled = TRUE;
	return 0;
}

LRESULT CLoginWnd::MobileVerify(UINT uMsg, MobileVerifyResult *res, int error_code, BOOL& bHandled)
{
	if (!res)
	{
		ShowLoginResult("获取短信验证码失败!");
		return 0;
	}
		
	if (0 == res->nCode)
	{
		if (10 == res->nOption)
		{
			CVerifyWnd *pWnd = new CVerifyWnd;
			pWnd->Create(NULL, _T("MicroChat"), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
			pWnd->ShowModal();
			delete pWnd;
			pWnd = NULL;
		}
		else if (13 == res->nOption)
		{
			//跳过
		}
		else
		{
			ShowLoginResult("短信授权成功,请重新登录!");
		}
	}
	else
	{
		if (13 == res->nOption)
		{
			//收到消息后再登录
			
		}
		else
		{
			ShowLoginResult("短信授权失败,错误码:%d,错误信息:%s", res->nCode, res->szMsgResult);
		}		
	}
	
	SAFE_DELETE(res);
	
	bHandled = TRUE;
	return 0;
}

void CLoginWnd::InitWindow()
{
	SetWindowText(m_hWnd, L"微信");
	SetIcon(IDI_SMALL);
	CenterWindow();

	//加载SDK
	StartSDK();

	//注册回调函数
	RegisterCgiCallBack(SDKCallBack);
}

void CLoginWnd::Notify( TNotifyUI& msg )
{
	if( msg.sType == _T("click") ) 
	{
		if (_tcsicmp(msg.pSender->GetName(), _T("closebtn")) == 0)
		{
			Close();
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("minbtn")) == 0)
		{
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
		}
		else if( msg.pSender->GetName() == _T("login") ) 
		{		
			CEditUI *pUser = (CEditUI *)m_PaintManager.FindControl(L"username");
			CEditUI *pPwd = (CEditUI *)m_PaintManager.FindControl(L"password");
			if (pUser && pPwd)
			{
				CStringA user = CW2A(pUser->GetText());
				CStringA pwd = CW2A(pPwd->GetText());

				if (user.IsEmpty())
				{
					ShowLoginResult("用户名不能为空");
				}
				else if (pwd.IsEmpty())
				{
					ShowLoginResult("密码不能为空");
				}
				else
				{
					Login(user.GetBuffer(), user.GetLength(), pwd.GetBuffer(), pwd.GetLength());
				}				
			}
		}
	}

	__super::Notify(msg);
}


void CLoginWnd::ShowLoginResult(const char *szLog, ...)
{
	va_list va;
	va_start(va, szLog);
	char buffer[MAX_BUF] = { 0 };
	vsprintf(buffer, szLog, va);
	va_end(va);

	CLabelUI *pLabel = (CLabelUI *)m_PaintManager.FindControl(L"loginMsg");
	if (pLabel)
	{
		pLabel->SetText(CA2W(buffer));
		pLabel->SetToolTip(CA2W(buffer));
	}
}

CLoginWnd * CLoginWnd::GetInstance()
{
	if (!m_Instance)
	{
		m_Instance = new CLoginWnd();
	}

	return m_Instance;
}

LRESULT CLoginWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT lRes = 0;
	bHandled = FALSE;

	switch (uMsg)
	{
	case WM_SHOW_LOGIN_RESULT:
	{
		if (wParam)
		{
			LoginResult *res = (LoginResult *)wParam;

			CString str;
			str.Format(L"错误码:%d,登录失败原因:%s", lParam, CA2W(res->szMsg));
			CLabelUI *pLabel = (CLabelUI *)m_PaintManager.FindControl(L"loginMsg");
			if (pLabel)
			{
				pLabel->SetText(str);
				pLabel->SetToolTip(str);
			}

			SAFE_DELETE(res);
		}
		break;
	}
	case WM_LOGIN_SCAN_QRCODE:
		ScanQrcode(uMsg, (LoginResult *)wParam, lParam, bHandled);
		break;
	case WM_LOGIN_SUCC:
	{
		ShowLoginResult("登陆成功!");
		
		//登录成功,隐藏登录窗口,显示主窗口
		ShowWindow(SW_HIDE);
		pMainWnd->Create(NULL, _T("MicroChat"), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
		pMainWnd->ShowModal();
		delete pMainWnd;

		//准备退出主程序
		Close();

		break;
	}
	case WM_MOBILE_VERIFY:
	{
		MobileVerify(uMsg, (MobileVerifyResult *)wParam, lParam, bHandled);
		break;
	}
		
	}


	return 0;
}

void CLoadingWnd::InitWindow()
{
	SetIcon(IDI_SMALL);
	CenterWindow();

	//loading界面 逐渐缩小 3s后退出
	SetTimer(m_hWnd,0,50,NULL);
}

LRESULT CLoadingWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT lRes = 0;
	bHandled = FALSE;

	switch (uMsg)
	{
		case WM_TIMER: 
			lRes = OnTimer(uMsg, wParam, lParam, bHandled);
			break;
	}

	return 0;
}

LRESULT CLoadingWnd::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	static int s_nWaitTime = 20;

	//延时1s
	if (s_nWaitTime > 0)
	{
		s_nWaitTime--;
		bHandled = TRUE;
		return 0;
	}

//每次缩小比率
#define ZOOM_RATE  0.9

	RECT rc;
	GetClientRect(m_hWnd, &rc);
	
	if (rc.bottom * ZOOM_RATE > 100)
	{
		::SetWindowPos(m_hWnd, NULL, 0, 0, (int)(rc.right * ZOOM_RATE), (int)(rc.bottom * ZOOM_RATE), SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		CenterWindow();
	}
	else
	{
		Close();
	}
	
	bHandled = TRUE;
	return 0;
}

void CVerifyWnd::InitWindow()
{
	CenterWindow();
}

LRESULT CVerifyWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

void CVerifyWnd::Notify(TNotifyUI& msg)
{
	CString strName = msg.pSender->GetName();
	if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (strName == L"closebutton")
		{
			Close();
		}
		else if (strName == L"send")
		{
			CControlUI *pControl = m_PaintManager.FindControl(L"code");
			if (pControl)
			{
				CStringA strCode = CW2A(pControl->GetText());
				SendMobileVerifyCode(strCode, strCode.GetLength());
			}
		}
	}
}
