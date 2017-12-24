#pragma once
#include "sqlite3.h"
#include <atlstr.h>
#include <list>
#include <windows.h>

using namespace std;

/** @brief 使用同步模式(效率较低,但可以保证时序) **/
//#define USE_SYNC_MODE

class Sql3
{
public:
	Sql3();
	~Sql3();

	/** @brief 非独占打开数据库,若不存在则创建数据库和表 **/
	void Init(LPCSTR strDbNameUtf8);

	/** @brief 关闭数据库,清空Sql队列 **/
	void Close();

	/** @brief 数据库操作:增、删、改、查 **/
	int ExecSQL(LPCSTR strSQLUtf8, sqlite3_callback pCallBack = NULL, void *pUserArg = NULL);

	/** @brief 异步执行Sql语句 **/
	int ExecSQL(LPCTSTR strSql);

	/** @brief 开启事务(提高插入数据速度) **/
	int Begin();

	/** @brief 关闭事务(真正的IO操作,若程序异常则之前所有操作失败!) **/
	int Commit();

	/** @brief 数据库建表 **/
	virtual int CreateTable() = 0;

	list<CString>					m_SqlList;		/**< Sql语句队列  >**/
	CRITICAL_SECTION				m_cs_sql;		/**< Sql队列锁  >**/

	/** @brief 异步执行Sql线程 **/
	static unsigned int __stdcall ExecSQLThread(void *pSql);
	
	void Lock(CRITICAL_SECTION &cs)		{ EnterCriticalSection(&cs); }
	void UnLock(CRITICAL_SECTION &cs)	{ LeaveCriticalSection(&cs); }

	CRITICAL_SECTION	m_cs;			/**< 数据库访问锁 >**/

private:
	CStringA			m_strDbNameUtf8;				/**< 当前操作的数据库文件名(UTF8格式) >**/
	sqlite3*			m_pDB		= NULL;				/**< 当前操作的数据库对象指针 >**/
	HANDLE				m_handle	= NULL;				/**< 用于销毁线程 >**/
};

/*
====================================================================================
====================================================================================
*/

CStringA UnicodeToUtf8(CString strUnicode);
CString Utf8ToUnicode(CStringA strUtf8);

#define SQL3REPLACE(strSql3)	strSql3.Replace(L"\'",L"\'\'")		/**< Sql3语句中单引号要转义为两个单引号处理 >**/
#define CW2UTF8(strUnicode)		UnicodeToUtf8(strUnicode)
#define CUTF82W(strUtf8)		Utf8ToUnicode(strUtf8)