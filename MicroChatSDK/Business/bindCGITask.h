#pragma once

#include "Wrapper/CGITask.h"
#include "mars/boost/weak_ptr.hpp"
#include "fun.h"
#include "MakeHeader.h"


class BindCGITask;


class BindCGITask : public CGITask,public BaseHeader
{
public:
	virtual bool Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select);
	virtual int Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select);

	string m_strPhoneNum;
	string m_strAuthTicket;
	int    m_nOptionType = 10;
	string m_strVerifyCode;
private:
	void GenAesKey();			//握手包aes(仅用于本次通讯,登陆成功后使用ECDH协商出的session key)
	string MakeReq();
	
	virtual string MakeHeader(int cgiType, int nLenProtobuf, int nLenCompressed, int nLenRsa);
	virtual string UnPackHeader(string pack);

	string		m_strAesKey;
	int			m_nAesKeyLen = 16;

	CGI_TYPE m_nCgiType = CGI_TYPE_BIND;
};