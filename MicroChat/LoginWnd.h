#pragma once


enum WM_LOGIN
{
	//显示登录结果
	WM_SHOW_LOGIN_RESULT = WM_USER + 1000,

	//弹出扫码授权页面
	WM_LOGIN_SCAN_QRCODE,

	//弹出输入短信授权界面
	WM_MOBILE_VERIFY,

	//登录成功
	WM_LOGIN_SUCC,

	
};

class CLoginWnd : public WindowImplBase
{
public:
	virtual void InitWindow();
	virtual void Notify(TNotifyUI& msg);

	static CLoginWnd *GetInstance();
protected:
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual CDuiString GetSkinFolder() { return _T("skin"); };
	virtual CDuiString GetSkinFile() { return _T("login.xml"); };
	virtual LPCTSTR GetWindowClassName(void) const { return _T("MicroChat_Login"); };

	//显示登录失败原因
	void ShowLoginResult(const char *szLog, ...);
private:
	static CLoginWnd *		m_Instance;

	//参数异常,弹框退出
	void QuitWithMsg(CString strMsg, DWORD dwTimeout = 3 * 1000);

	//扫码授权
	LRESULT ScanQrcode(UINT uMsg, LoginResult *res, int error_code, BOOL& bHandled);

	//短信授权
	LRESULT MobileVerify(UINT uMsg, MobileVerifyResult *res, int error_code, BOOL& bHandled);
};
#define pLoginWnd (CLoginWnd::GetInstance())


class CLoadingWnd : public WindowImplBase
{
public:
	virtual void InitWindow();


protected:
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	virtual CDuiString GetSkinFolder() { return _T("skin"); };
	virtual CDuiString GetSkinFile() { return _T("loading.xml"); };
	virtual LPCTSTR GetWindowClassName(void) const { return _T("MicroChat_Loading"); };
};

class CVerifyWnd : public WindowImplBase
{
public:
	virtual void InitWindow();

protected:
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual void Notify(TNotifyUI& msg);

	virtual CDuiString GetSkinFolder() { return _T("skin"); };
	virtual CDuiString GetSkinFile() { return _T("sendMobileCode.xml"); };
	virtual LPCTSTR GetWindowClassName(void) const { return _T("MicroChat_Verify"); };
};