#include "stdafx.h"
#include "GetContactCGITask.h"
#include <mars/comm/windows/projdef.h>
#include "proto/generate/searchcontact.pb.h"
#include "business/crypto/mmCryptInterface.h"
#include "AuthInfo.h"

bool GetContactCGITask::Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select)
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

int GetContactCGITask::Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select)
{
	_error_code = CGI_ERR_UNKNOWN;

	m_res = new SearchContactResult;
	NEW_ERR(m_res);

	string body = UnPackHeader(string((const char *)_inbuffer.Ptr(), _inbuffer.Length()));

	if (!body.size())
	{
		LOG("封包异常，请按mm协议正确发送请求!\r\n", _inbuffer.Length());
		_error_code = CGI_CODE_UNPACK_ERR;
		return 0;
	}

	string RespProtobuf;

	if (5 == m_nDecryptType)
	{
		if (m_bCompressed)
		{
			RespProtobuf = aes_uncompress(pAuthInfo->m_Session, body, m_nLenRespProtobuf);
		}
		else
		{
			RespProtobuf = aes_nouncompress(pAuthInfo->m_Session, body);
		}

		if (RespProtobuf.size())
		{
			_error_code = CGI_CODE_OK;
		}

		com::tencent::mars::microchat::proto::GetContactResponse resp;
		bool bRet = resp.ParsePartialFromArray(RespProtobuf.c_str(), RespProtobuf.size());
		if (bRet)
		{
			((SearchContactResult *)m_res)->nCode = resp.tag1().code();
			strcpy_s(((SearchContactResult *)m_res)->szMsgResult, Utf82CStringA(resp.tag1().result().strresult().c_str()));
			strcpy_s(((SearchContactResult *)m_res)->szV1_Name, resp.tag3().v1().v1_name().c_str());
		}
		else
		{
			_error_code = CGI_CODE_PARSE_PROTOBUF_ERR;
			return 0;
		}
	}
	else
	{
		_error_code = CGI_CODE_DECRYPT_ERR;
		return 0;
	}
	
	return 0;
}

std::string GetContactCGITask::MakeMsgReq()
{
	string req;

	com::tencent::mars::microchat::proto::GetContactRequest newsearchReq;
	com::tencent::mars::microchat::proto::GetContactRequest_LoginInfo *pLogin = new com::tencent::mars::microchat::proto::GetContactRequest_LoginInfo;
	com::tencent::mars::microchat::proto::GetContactRequest__SearchName *pSearchName = new com::tencent::mars::microchat::proto::GetContactRequest__SearchName;
	com::tencent::mars::microchat::proto::GetContactRequest__tag7 *pTag7 = new com::tencent::mars::microchat::proto::GetContactRequest__tag7;

	pLogin->set_aeskey(pAuthInfo->m_Session.c_str());
	pLogin->set_uin(pAuthInfo->m_uin);
	pLogin->set_guid(string(pAuthInfo->m_guid_15.c_str(), 16));
	pLogin->set_clientver(pAuthInfo->m_ClientVersion);
	pLogin->set_androidver(pAuthInfo->m_androidVer);
	pLogin->set_unknown3(3);

	pSearchName->set_name(m_searchName);

	pTag7->set_tag1("");

	newsearchReq.set_allocated_login(pLogin);
	newsearchReq.set_allocated_name(pSearchName);
	newsearchReq.set_allocated_tag7(pTag7);
	newsearchReq.set_tag2(1);
	newsearchReq.set_tag4(0);
	newsearchReq.set_tag6(1);
	newsearchReq.set_tag8(0);

	newsearchReq.SerializeToString(&req);

	newsearchReq.release_login();
	newsearchReq.release_name();
	newsearchReq.release_tag7();
	delete pLogin;
	delete pSearchName;
	delete pTag7;

	return req;
}
