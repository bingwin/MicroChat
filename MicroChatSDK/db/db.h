#pragma once
#include <time.h>
#include "sql.h"
#include "interface.h"


/** @brief synckey表字段  **/
enum DB_INDEX_SYNCKEY
{
	DB_INDEX_SYNCKEY_KEY = 0,				/**< synckey  >**/
	DB_INDEX_RECORD_CNT,					/**< synckey表字段数量  >**/
};

/** @brief contact表字段  **/
enum DB_INDEX_CONTACT
{
	DB_INDEX_CONTACT_WXID = 0,				/**< wxid  >**/
	DB_INDEX_CONTACT_NICKNAME,				/**< nickname  >**/
	DB_INDEX_CONTACT_HEADICON_URL,			/**< 头像url  >**/
	DB_INDEX_CONTACT_V1_NAME,				/**< V1数据  >**/

	DB_INDEX_CONTACT_CNT,					/**< contact表字段数量  >**/
};

/** @brief msg表字段  **/
enum DB_INDEX_MSG
{
	DB_INDEX_MSG_SVRID = 0,				/**< svrid  >**/
	DB_INDEX_MSG_UTC,					/**< utc >**/
	DB_INDEX_MSG_CREATETIME,			/**< 消息发送者创建时间 >**/	
	DB_INDEX_MSG_TYPE,					/**< 消息类型 >**/
	DB_INDEX_MSG_FROM,				    /**< 发消息人wxid  >**/
	DB_INDEX_MSG_TO,				    /**< 收消息人wxid  >**/
	DB_INDEX_MSG_CONTENT,				/**< 消息内容  >**/

	DB_INDEX_MSG_CNT,					/**< contact表字段数量  >**/
};

/** @brief DownloadManager数据库 **/
class CMicroChatDB : public Sql3
{
public:
	/** @brief 建表 **/
	virtual int CreateTable();

	//清空数据库
	void ClearDB();

	//获取同步key
	string GetSyncKey();

	//刷新同步key
	void UpdateSyncKey(string strSyncKey);

	//载入联系人
	void LoadContact();

	//载入历史消息
	void LoadMsgRecord();

	//插入新消息
	void AddMsg(NewMsg *pMsg);

	static CMicroChatDB *GetInstance();
private:
	//数据库查询回调函数
	static int GetSyncKeyCallBack(void *lpUserArg, int argc, char **argv, char **lpszValue);

	//联系人查询回调函数
	static int LoadContactCallBack(void *lpUserArg, int argc, char **argv, char **lpszValue);

	//查询消息记录回调函数
	static int LoadMsgRecordCallBack(void *lpUserArg, int argc, char **argv, char **lpszValue);

	static CMicroChatDB *m_Instance;
};
#define pMicroChatDb	(CMicroChatDB::GetInstance())   

