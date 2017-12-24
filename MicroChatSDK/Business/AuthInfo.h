#pragma once
#include <string>
#include "db/db.h"

class CAuthInfo
{
public:
	CAuthInfo()
	{
		InitializeCriticalSection(&m_cs_syncKey);
	}

	string	m_UserName;
	string	m_WxId;
	DWORD   m_uin = 0;
	string	m_Alias;
	string	m_Session;	
	DWORD   m_ClientVersion;
	string  m_guid_15;
	string  m_guid;
	string  m_androidVer;
	string  m_launguage;
	string  m_cookie;

	string GetSyncKey();
	void SetSyncKey(string strSyncKey);


	static CAuthInfo *GetInstance();

	//获取短信验证码凭据
	string m_mobilecode_authticket;
	//接受短信号码(当前默认使用登录账号)
	string m_mobileNum;

private:
	static CAuthInfo * m_Instance;

	CRITICAL_SECTION   m_cs_syncKey;
};

#define pAuthInfo (CAuthInfo::GetInstance())