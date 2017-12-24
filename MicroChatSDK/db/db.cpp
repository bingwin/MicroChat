#include "stdafx.h"
#include "db.h"
#include "Business\fun.h"
#include "Business\AuthInfo.h"
#include "Wrapper\NetworkService.h"

static CStringA Utc2BeijingTime(DWORD dwUtc)
{
	CStringA strTime;

	time_t utc = dwUtc;
	struct tm *tmdate = localtime(&utc);

	strTime.Format("%d-%02d-%02d %02d:%02d:%02d",
		tmdate->tm_year + 1900, tmdate->tm_mon + 1, tmdate->tm_mday, tmdate->tm_hour, tmdate->tm_min, tmdate->tm_sec);

	return strTime;
}


CMicroChatDB *CMicroChatDB::m_Instance = NULL;
CMicroChatDB * CMicroChatDB::GetInstance()
{
	if (!m_Instance)
	{
		m_Instance = new CMicroChatDB;
	}

	return m_Instance;
}


int CMicroChatDB::CreateTable()
{
	int ret = SQLITE_OK;

	//同步key表
	CStringA str = "create table synckey(key varchar(4096));";

	ret = ExecSQL(str);
	if (SQLITE_OK != ret)
	{
		LOG("[CSynckeyDB::CreateTable]synckey建表失败!");
	}

	//好友列表
	str = "create table contact(wxid varchar(1024),nickname varchar(1024),headicon varchar(1024),v1_name varchar(1024));";

	ret = ExecSQL(str);
	if (SQLITE_OK != ret)
	{
		LOG("[CSynckeyDB::CreateTable]contact建表失败!");
	}

	//历史消息记录表
	str = "create table msg(svrid bigint,utc integer,createtime varchar(1024),fromWxid varchar(1024),toWxid varchar(1024),type integer,content text(65535));";

	ret = ExecSQL(str);
	if (SQLITE_OK != ret)
	{
		LOG("[CSynckeyDB::CreateTable]msg建表失败!");
	}

	return SQLITE_OK;
}

void CMicroChatDB::ClearDB()
{
	CStringA strSql;
	strSql.Format("delete from msg;");
	ExecSQL(strSql,NULL,NULL);

	strSql.Format("delete from contact;");
	ExecSQL(strSql, NULL, NULL);

	strSql.Format("delete from synckey;");
	ExecSQL(strSql, NULL, NULL);
}

string CMicroChatDB::GetSyncKey()
{
	char szSyncKey[4096] = { 0 };

	CString strSql;
	strSql.Format(L"select * from synckey;");
	ExecSQL(CW2UTF8(strSql), &CMicroChatDB::GetSyncKeyCallBack, szSyncKey);

	return string(szSyncKey,strlen(szSyncKey));
}

void CMicroChatDB::UpdateSyncKey(string strSyncKey)
{
	//清空表
	CString strSql;
	strSql.Format(L"delete from synckey;");
	ExecSQL(CW2UTF8(strSql), NULL, NULL);

	//插入记录
	strSql.Format(L"insert into synckey(key) values('%s');", CA2W(strSyncKey.c_str()));
	ExecSQL(CW2UTF8(strSql), NULL, NULL);
}


void CMicroChatDB::LoadContact()
{
	NewInitResult *res = new NewInitResult;
	res->ppContanct = new ContactInfo *[1024];
	
	CString strSql;
	strSql.Format(L"select * from contact;");
	ExecSQL(CW2UTF8(strSql), &CMicroChatDB::LoadContactCallBack, res);
	
	for (int i=res->dwSize;i<1024;i++)
	{
		SafeFree(res->ppContanct[i]);
	}

	//通过newinit回调函数将结果通知上层
	pNetworkService.DoCallBack(res, CGI_TYPE_NEWINIT, 0, CGI_CODE_OK);
}

void CMicroChatDB::LoadMsgRecord()
{
	NewSyncResult *res = new NewSyncResult;
	res->ppNewMsg = new NewMsg *[1024];

	//TODO:这里sql语句应该先select出所有wxid,然后根据每个wxid limit出最近n条消息记录;
	//这里意思一下,默认只显示1条历史消息
	CString strSql;
	strSql.Format(L"select *,count(fromWxid) from msg group by fromWxid;");
	ExecSQL(CW2UTF8(strSql), &CMicroChatDB::LoadMsgRecordCallBack, res);

	for (int i = res->dwSize; i < 1024; i++)
	{
		SafeFree(res->ppNewMsg[i]);
	}

	//通过newsync回调函数将消息记录回调给上层
	pNetworkService.DoCallBack(res, CGI_TYPE_NEWSYNC, 0, CGI_CODE_OK);
}

void CMicroChatDB::AddMsg(NewMsg *pMsg)
{
	if (!pMsg)	return;

	//插入消息记录
	CStringA strSql;
	strSql.Format("insert into msg(svrid,utc,createtime,fromWxid,toWxid,type,content) values('%lld','%d','%s','%s','%s','%d','%s');", 
		pMsg->svrid, pMsg->utc,Utc2BeijingTime(pMsg->utc),pMsg->szFrom,pMsg->szTo,pMsg->nType,CStringA2Utf8(pMsg->szContent));
	ExecSQL(strSql, NULL, NULL);
}

int CMicroChatDB::GetSyncKeyCallBack(void *lpUserArg, int argc, char **argv, char **lpszValue)
{
	//字段数量不符,放弃解析该任务
	if (DB_INDEX_RECORD_CNT != argc)		return SQLITE_OK;
	
	//TODO:没做异常处理
	strcpy((char *)lpUserArg, argv[DB_INDEX_SYNCKEY_KEY]);

	return 0;
}

int CMicroChatDB::LoadContactCallBack(void *lpUserArg, int argc, char **argv, char **lpszValue)
{
	//字段数量不符,放弃解析该任务
	if (DB_INDEX_CONTACT_CNT != argc)		return SQLITE_OK;
	if (!lpUserArg)	return SQLITE_OK;
	
	NewInitResult *res = (NewInitResult *)lpUserArg;

	ContactInfo *pInfo = new ContactInfo;
	strcpy_s(pInfo->wxid, argv[DB_INDEX_CONTACT_WXID]);
	strcpy_s(pInfo->nickName, argv[DB_INDEX_CONTACT_NICKNAME]);	//注意Utf8格式存储
	strcpy_s(pInfo->headicon, argv[DB_INDEX_CONTACT_HEADICON_URL]);
	strcpy_s(pInfo->v1_Name, argv[DB_INDEX_CONTACT_V1_NAME]);
	res->ppContanct[res->dwContanct++] = pInfo;
	res->dwSize++;

	return 0;
}

int CMicroChatDB::LoadMsgRecordCallBack(void *lpUserArg, int argc, char **argv, char **lpszValue)
{
	//字段数量不符,放弃解析该任务
	if ((DB_INDEX_MSG_CNT+1) != argc)		return SQLITE_OK;
	if (!lpUserArg)	return SQLITE_OK;

	NewSyncResult *res = (NewSyncResult *)lpUserArg;

	NewMsg *pMsg = new NewMsg;

	for (int i=0;i<argc;i++)
	{
		if (!argv[i])	continue;
		
		CStringA strNameValue = lpszValue[i];

		if (strNameValue == "fromWxid")
			strcpy_s(pMsg->szFrom, argv[i]);
		else if (strNameValue == "toWxid")
			strcpy_s(pMsg->szTo, argv[i]);
		else if (strNameValue == "content")
			strcpy_s(pMsg->szContent, Utf82CStringA(argv[i]));
		else if (strNameValue == "type")
			pMsg->nType = atoi(argv[i]);
		else if (strNameValue == "utc")
			pMsg->utc = atoll(argv[i]);
		else if (strNameValue == "svrid")
			pMsg->svrid = atoll(argv[DB_INDEX_MSG_SVRID]);
	}
	

	res->ppNewMsg[res->dwNewMsg++] = pMsg;
	res->dwSize++;
	
	return 0;
}

