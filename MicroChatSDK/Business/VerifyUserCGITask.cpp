#include "stdafx.h"
#include "VerifyUserCGITask.h"
#include <mars/comm/windows/projdef.h>
#include "proto/generate/verifyuser.pb.h"
#include "business/crypto/mmCryptInterface.h"
#include "AuthInfo.h"
#include "Wrapper/NetworkService.h"

bool VerifyUserCGITask::Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select)
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

int VerifyUserCGITask::Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select)
{
	_error_code = CGI_ERR_UNKNOWN;

	m_res = new VerifyUserResult;
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

		com::tencent::mars::microchat::proto::VerifyUserResponse resp;
		bool bRet = resp.ParsePartialFromArray(RespProtobuf.c_str(), RespProtobuf.size());
		if (bRet)
		{
			((VerifyUserResult *)m_res)->nCode = resp.tag1().code();
			strcpy_s(((VerifyUserResult *)m_res)->szMsgResult, Utf82CStringA(resp.tag1().result().strresult().c_str()));
		
			//加好友确认
			if (1 == m_tag2 && resp.tag1().code())
			{
				VerifyUserCGITask * task = new VerifyUserCGITask();
				task->m_userName = m_userName;
				task->m_v2Name = m_v2Name;
				task->m_sayContent = m_sayContent;
				task->channel_select_ = ChannelType_ShortConn;
				task->cmdid_ = CGI_TYPE_VERIFYUSER;
				task->cgi_ = CGI_VERIFYUSER;
				task->cgitype_ = CGI_TYPE_VERIFYUSER;
				task->host_ = SHORTLINK_HOST;
				task->m_tag2 = 2;
				pNetworkService.startTask(task);
			}			
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

std::string VerifyUserCGITask::MakeMsgReq()
{
	string req;

	com::tencent::mars::microchat::proto::VerifyUserRequest newsearchReq;
	com::tencent::mars::microchat::proto::VerifyUserRequest_LoginInfo *pLogin = new com::tencent::mars::microchat::proto::VerifyUserRequest_LoginInfo;
	com::tencent::mars::microchat::proto::VerifyUserRequest__UserName *pUserName = new com::tencent::mars::microchat::proto::VerifyUserRequest__UserName;

	pLogin->set_aeskey(pAuthInfo->m_Session.c_str());
	pLogin->set_uin(pAuthInfo->m_uin);
	pLogin->set_guid(string(pAuthInfo->m_guid_15.c_str(),16));
	pLogin->set_clientver(pAuthInfo->m_ClientVersion);
	pLogin->set_androidver(pAuthInfo->m_androidVer);
	pLogin->set_unknown3(0);

	pUserName->set_tag1(m_userName.c_str());
	pUserName->set_tag2("");
	pUserName->set_tag3(m_v2Name.c_str());
	pUserName->set_tag4(0);
	if (1 == m_tag2)
	{
		pUserName->set_tag5("");
	}
	pUserName->set_tag8(0);
	pUserName->set_tag9("");

	newsearchReq.set_allocated_login(pLogin);
	newsearchReq.set_tag2(m_tag2);
	newsearchReq.set_tag3(1);
	newsearchReq.set_allocated_name(pUserName);
	if (2 == m_tag2)
		newsearchReq.set_content(m_sayContent.c_str());
	else
		newsearchReq.set_content("");	
	newsearchReq.set_tag6(1);

	//Unknown flag,但是填写错误会添加失败
	char szTag7[2] = {0xf};
	newsearchReq.set_tag7(string(szTag7,1));

	newsearchReq.SerializeToString(&req);

	newsearchReq.release_login();
	newsearchReq.release_name();
	delete pLogin;
	delete pUserName;

	return req;
}
