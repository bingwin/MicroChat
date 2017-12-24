#pragma once

enum WM_FRIEND
{
	//搜索联系人
	WM_FRIEND_SEARCH_CONTACT = WM_USER + 1000,

	//添加好友
	WM_FRIEND_ADD_NEW_FRIEND,
};

class CFriendWnd : public WindowImplBase
{
public:
	virtual void InitWindow();
	virtual void Notify(TNotifyUI& msg);

	static CFriendWnd *GetInstance();
protected:
	virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& /*bHandled*/);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual CDuiString GetSkinFolder() { return _T("skin"); };
	virtual CDuiString GetSkinFile() { return _T("friend.xml"); };
	virtual LPCTSTR GetWindowClassName(void) const { return _T("MicroChat_Test_Friend"); };
	virtual LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnSearchContact(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnAddNewFriend(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
	static CFriendWnd *		m_Instance;

	void ShowText(CString str,bool bClear = TRUE);
};
#define pFriendWnd (CFriendWnd::GetInstance())