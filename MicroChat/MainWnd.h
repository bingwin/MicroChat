#pragma once

enum WM_MAIN
{	
	//新增好友列表
	WM_MAIN_ADD_FRIEND = WM_USER + 0xFFFF,
	
	//接收新消息
	WM_MAIN_ADD_NEW_MSG,

	//网络通信错误
	WM_MAIN_ERR,

	//弹框退出
	WM_MAIN_FORCE_QUIT,
};

struct ContactInfo;

//联系人List节点中存储的相关数据
struct CNodeInfo
{
	//聊天记录
	list<CListContainerElementUI*>	msgRecordList;

	//联系人详细信息
	ContactInfo contactinfo;
};

class CMainWnd : public WindowImplBase
{
public:
	virtual void InitWindow();
	virtual void Notify(TNotifyUI& msg);

	static CMainWnd* GetInstance();
protected:
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual CDuiString GetSkinFolder() { return _T("skin"); }
	virtual CDuiString GetSkinFile() { return _T("main.xml"); }
	virtual LPCTSTR GetWindowClassName(void) const { return _T("MicroChat_Main"); }

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnSelectChanged(TNotifyUI &msg);
	virtual void OnItemClick(TNotifyUI &msg);

	//当前聊天窗口的对象的Node节点
	CListContainerElementUI *m_pCurSelContactControlUI = NULL;

private:
	CListContainerElementUI* NewOneFrientListNode(ContactInfo *pInfo);
	CListContainerElementUI* NewOneChatListNodeMe();
	CListContainerElementUI* NewOneChatListNodeOther();

	//立即刷新聊天记录窗口
	void UpdateRecord();

	
	static CMainWnd *		m_Instance;

	//参数异常,弹框退出
	void QuitWithMsg(CString strMsg,DWORD dwTimeout = 3*1000);

};
#define pMainWnd (CMainWnd::GetInstance())