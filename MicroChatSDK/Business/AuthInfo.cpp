#include "stdafx.h"
#include "AuthInfo.h"
#include "fun.h"


CAuthInfo *CAuthInfo::m_Instance = NULL;

string CAuthInfo::GetSyncKey()
{
	string strSyncKey;

	EnterCriticalSection(&m_cs_syncKey);
	strSyncKey = pMicroChatDb->GetSyncKey();
	LeaveCriticalSection(&m_cs_syncKey);

	return Hex2bin(strSyncKey.c_str());
}

void CAuthInfo::SetSyncKey(string strSyncKey)
{
	EnterCriticalSection(&m_cs_syncKey);
	pMicroChatDb->UpdateSyncKey(Bin2Hex(strSyncKey));
	LeaveCriticalSection(&m_cs_syncKey);
}

CAuthInfo *CAuthInfo::GetInstance()
{
	if (!m_Instance)
	{
		m_Instance = new CAuthInfo();
	}

	return m_Instance;
}