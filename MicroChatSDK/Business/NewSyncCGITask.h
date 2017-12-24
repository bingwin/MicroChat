#pragma once

#include "Wrapper/CGITask.h"
#include "mars/boost/weak_ptr.hpp"
#include "fun.h"
#include "MakeHeader.h"


class NewSyncCGITask;


class NewSyncCGITask : public CGITask, public BaseHeader
{
public:

	virtual bool Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select);
	virtual int Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select);

	static bool IsSyncing();		//是否正在同步(同一时间只允许一个同步任务在执行)
	static bool IsNeedReSync();			//一次同步任务结束后,是否需要立即再次同步
	
	static CRITICAL_SECTION s_cs;
	static bool s_bSyncing;			//TRUE表示已有同步任务即将执行,不需要再投递同步任务
	static bool s_bNeedReSync;		//TRUE表示当前同步任务结束后,需要立即启动一次同步任务
private:
	string MakeNewSyncReq();
	CGI_TYPE m_nCgiType = CGI_TYPE_NEWSYNC;

};