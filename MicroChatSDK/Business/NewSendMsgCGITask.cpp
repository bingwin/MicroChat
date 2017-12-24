#include "stdafx.h"
#include "NewSendMsgCGITask.h"
#include <mars/comm/windows/projdef.h>
#include "proto/generate/NewSendMsg.pb.h"
#include "business/crypto/mmCryptInterface.h"
#include "AuthInfo.h"
#include <time.h>

bool NewSendCGITask::Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select)
{
	string reqProtobuf = MakeMsgReq();

	DWORD dwCompressed = 0;
	string body = nocompress_aes(pAuthInfo->m_Session, reqProtobuf);
	if (!body.size())	return FALSE;

	string header = MakeHeader(BaseHeader::s_cookie, m_nCgiType, reqProtobuf.size(), reqProtobuf.size());
	
	string req = header + body;

	_outbuffer.AllocWrite(req.size());
	_outbuffer.Write(req.c_str(), req.size());

	return TRUE;
}

int NewSendCGITask::Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select)
{
	//TODO  解包判断发送是否成功

	//发送成功后插入数据库
	NewMsg msg;
	msg.utc = time(NULL);
	strcpy_s(msg.szFrom, pAuthInfo->m_WxId.c_str());
	strcpy_s(msg.szTo, m_ToId.c_str());
	strcpy_s(msg.szFrom, pAuthInfo->m_WxId.c_str());
	msg.nType = 1;
	strcpy_s(msg.szContent, m_content.c_str());

	pMicroChatDb->AddMsg(&msg);
	
	return 0;
}

std::string NewSendCGITask::MakeMsgReq()
{
	string req;

	com::tencent::mars::sample::proto::NewSendMsgRequest newsendReq;
	com::tencent::mars::sample::proto::NewSendMsgRequest_ChatInfo *pInfo = new com::tencent::mars::sample::proto::NewSendMsgRequest_ChatInfo;
	com::tencent::mars::sample::proto::NewSendMsgRequest_ChatInfo_ToId *pToid = new com::tencent::mars::sample::proto::NewSendMsgRequest_ChatInfo_ToId;
	pToid->set_id(m_ToId);
	pInfo->set_allocated_toid(pToid);
	pInfo->set_content(m_content);
	pInfo->set_unknown2(1);
	pInfo->set_utc(time(NULL));
	pInfo->set_unknown3(0x12345678);
	
	newsendReq.set_cnt(1);
	newsendReq.set_allocated_info(pInfo);

	newsendReq.SerializeToString(&req);

	return req;
}
