#include "stdafx.h"
#include "interface.h"
#include "Wrapper/CGITask.h"
#include "Wrapper/NetworkService.h"
#include "Business/define.h"
#include "Business/ManualAuthCGITask.h"
#include "Business/crypto/mmCryptInterface.h"
#include "Business/NewSendMsgCGITask.h"
#include "Business/NewInitCGITask.h"
#include "Business/NewSyncCGITask.h"
#include "db/db.h"
#include "Business/AuthInfo.h"
#include "Business/SearchContactCGITask.h"
#include "Business/GetContactCGITask.h"
#include "Business/VerifyUserCGITask.h"
#include "Business/bindCGITask.h"

//数据库文件名
#define DB_FILE_NAME		L"MicroChat-%08x.db"

void RegisterCgiCallBack(CGICallBack callback)
{
	pNetworkService.setCgiCallBack(callback);
}

void StartSDK()
{
	//初始化同步任务锁
	InitializeCriticalSection(&NewSyncCGITask::s_cs);
	
	pNetworkService.setClientVersion(LONGLINK_CLIENT_VER);

	vector<uint16_t> ports;
	ports.push_back(LONGLINK_PORT_80);
	pNetworkService.setLongLinkAddress(LONGLINK_HOST, ports, "");

	pNetworkService.setShortLinkPort(SHORTLINK_PORT);
	pNetworkService.start();
}

void StopSDK()
{
	pNetworkService.stop();
}

int Login(const char *user, int nLenUser, const char *pwd, int nLenPwd, bool bRandomDeivce /*= FALSE*/)
{
	ManualAuthCGITask* task = new ManualAuthCGITask();
	task->m_strUserName = string(user,nLenUser);
	task->m_strPwd = string(pwd, nLenPwd);

#if 1
	task->channel_select_ = ChannelType_LongConn;
	task->cmdid_ = MANUALAUTH;
	task->cgitype_ = CGI_TYPE_MANUALAUTH;
	task->cgi_ = CGI_MANUALAUTH;
	task->host_ = SHORTLINK_HOST;
#else
	task->channel_select_ = ChannelType_ShortConn;
	task->cmdid_ = MANUALAUTH;
	task->cgitype_ = CGI_TYPE_MANUALAUTH;
	task->cgi_ = CGI_MANUALAUTH;
	task->host_ = SHORTLINK_HOST;
#endif
	
	LOG("正在登录..............\r\n");

	return pNetworkService.startTask(task);
}

void RecvMobileVerifyCode()
{
	BindCGITask* task = new BindCGITask();

	task->m_nOptionType = 10;
	task->m_strPhoneNum = pAuthInfo->m_mobileNum;
	task->m_strAuthTicket = pAuthInfo->m_mobilecode_authticket;

	task->channel_select_ = ChannelType_ShortConn;
	task->cmdid_ = CGI_TYPE_BIND;
	task->cgitype_ = CGI_TYPE_BIND;
	task->cgi_ = CGI_BIND;
	task->host_ = SHORTLINK_HOST;

	pNetworkService.startTask(task);
}

int SendMobileVerifyCode(const char *code, int nLenCode)
{
	BindCGITask* task = new BindCGITask();

	task->m_nOptionType = 11;
	task->m_strPhoneNum = pAuthInfo->m_mobileNum;
	task->m_strAuthTicket = pAuthInfo->m_mobilecode_authticket;
	task->m_strVerifyCode = string(code,nLenCode);

	task->channel_select_ = ChannelType_ShortConn;
	task->cmdid_ = CGI_TYPE_BIND;
	task->cgitype_ = CGI_TYPE_BIND;
	task->cgi_ = CGI_BIND;
	task->host_ = SHORTLINK_HOST;

	return pNetworkService.startTask(task);
}

void LoadFromDB()
{
	//初始化数据库(按uin命名)
	CString strDbFileName;
	strDbFileName.Format(DB_FILE_NAME, pAuthInfo->m_uin);
	pMicroChatDb->Init(CW2UTF8(strDbFileName));
	
	//首先判断是否可以获取同步key
	string strSync = pAuthInfo->GetSyncKey();
	if (!strSync.size())
	{
		//同步key为空,按首次登录流程处理,清空数据库,然后重新初始化
		pMicroChatDb->ClearDB();
		NewInit();
	}
	else
	{
		//加载db信息(回调时会newsync一次)
		pMicroChatDb->LoadContact();
		pMicroChatDb->LoadMsgRecord();
	}
}

int NewInit()
{
	NewInitCGITask * task = new NewInitCGITask();

	task->channel_select_ = ChannelType_ShortConn;
	task->cmdid_ = CGI_TYPE_NEWINIT;
	task->cgi_ = CGI_NEWINIT;
	task->cgitype_ = CGI_TYPE_NEWINIT;
	task->host_ = SHORTLINK_HOST;
	return pNetworkService.startTask(task);
}

void NewSendMsg(const char *content, int nLenContent, const char *toWxid, int nLenToWxid)
{
	NewSendCGITask * task = new NewSendCGITask();

	task->m_content = string(content,nLenContent);
	task->m_ToId = string(toWxid,nLenToWxid);

	task->channel_select_ = ChannelType_ShortConn;
	task->cmdid_ = CGI_TYPE_NEWSENDMSG;
	task->cgi_ = CGI_NEWSENDMSG;
	task->host_ = SHORTLINK_HOST;
	pNetworkService.startTask(task);
}

void NewSync()
{
	if (!NewSyncCGITask::IsSyncing())
	{
		NewSyncCGITask * task = new NewSyncCGITask();

		task->channel_select_ = ChannelType_LongConn;
		task->cmdid_ = SEND_NEWSYNC_CMDID;
		task->cgi_ = CGI_NEWSYNC;
		task->cgitype_ = CGI_TYPE_NEWSYNC;
		task->host_ = SHORTLINK_HOST;
		pNetworkService.startTask(task);
	}	
}

void SearchContact(const char *szName, int nLenName)
{
#if 1
	SearchContactCGITask * task = new SearchContactCGITask();

	task->m_searchName = string(szName, nLenName);

	task->channel_select_ = ChannelType_ShortConn;
	task->cmdid_ = CGI_TYPE_SEARCHCONTACT;
	task->cgi_ = CGI_SEARCHCONTACT;
	task->cgitype_ = CGI_TYPE_SEARCHCONTACT;
	task->host_ = SHORTLINK_HOST;
	pNetworkService.startTask(task);
#else
	GetContactCGITask * task = new GetContactCGITask();

	task->m_searchName = string(szName, nLenName);

	task->channel_select_ = ChannelType_ShortConn;
	task->cmdid_ = CGI_TYPE_GETCONTACT;
	task->cgi_ = CGI_GETCONTACT;
	task->cgitype_ = CGI_TYPE_GETCONTACT;
	task->host_ = SHORTLINK_HOST;
	pNetworkService.startTask(task);
#endif
}

void AddNewFriend(const char *szUserName, int nLenUserName, const char *szV2Name, int nLenV2Name, const char *szContent, int nLenContent)
{
	VerifyUserCGITask * task = new VerifyUserCGITask();

	task->m_userName	= string(szUserName, nLenUserName);
	task->m_v2Name = string(szV2Name, nLenV2Name);
	task->m_sayContent	= string(szContent, nLenContent);

	task->channel_select_ = ChannelType_ShortConn;
	task->cmdid_ = CGI_TYPE_VERIFYUSER;
	task->cgi_ = CGI_VERIFYUSER;
	task->cgitype_ = CGI_TYPE_VERIFYUSER;
	task->host_ = SHORTLINK_HOST;
	pNetworkService.startTask(task);
}

void IsNeedVerify(const char *MobileNum, int nLenMobileNum)
{
	BindCGITask* task = new BindCGITask();

	task->m_nOptionType = 13;
	task->m_strPhoneNum = string(MobileNum,nLenMobileNum);
	task->m_strAuthTicket = "";

	task->channel_select_ = ChannelType_ShortConn;
	task->cmdid_ = CGI_TYPE_BIND;
	task->cgitype_ = CGI_TYPE_BIND;
	task->cgi_ = CGI_BIND;
	task->host_ = SHORTLINK_HOST;

	pNetworkService.startTask(task);
}

void SafeFree(void *p)
{
	__try 
	{
		if (!p)
		{
			delete p;
			p = NULL;
		}
	}
	__except (1)
	{
		LOG("[SafeFree]Crash!");
	}
}

