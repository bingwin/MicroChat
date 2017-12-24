#pragma once

#include "Wrapper/CGITask.h"
#include "fun.h"
#include "MakeHeader.h"

class NewInitCGITask;


class NewInitCGITask : public CGITask, public BaseHeader
{
public:
	virtual bool Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select);
	virtual int Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select);
	
private:
	string MakeReq();
};