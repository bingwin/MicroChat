#pragma once

#include "Wrapper/CGITask.h"
#include "mars/boost/weak_ptr.hpp"
#include "fun.h"
#include "MakeHeader.h"


class VerifyUserCGITask;


class VerifyUserCGITask : public CGITask, public BaseHeader
{
public:
	virtual bool Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select);
	virtual int Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select);

	string m_userName;
	string m_v2Name;
	string m_sayContent;

	int    m_tag2 = 1;
private:
	string MakeMsgReq();

	CGI_TYPE m_nCgiType = CGI_TYPE_VERIFYUSER;
};