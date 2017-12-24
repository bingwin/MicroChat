#include "stdafx.h"
#include "NewInitCGITask.h"

#include "proto/generate/NewInit.pb.h"
#include "business/crypto/mmCryptInterface.h"
#include "AuthInfo.h"
#include "MicroChat/fun.h"
#include "Wrapper/NetworkService.h"
#include "../generate/newsync.pb.h"
#include "ReportSyncKVCGITask.h"

bool NewInitCGITask::Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select)
{
	string reqProtobuf = MakeReq();

	DWORD dwCompressed = 0;
	string body = nocompress_aes(pAuthInfo->m_Session, reqProtobuf);
	if (!body.size())	return FALSE;

	string header = MakeHeader(BaseHeader::s_cookie, cmdid_, reqProtobuf.size(), reqProtobuf.size());

	string req = header + body;

	_outbuffer.AllocWrite(req.size());
	_outbuffer.Write(req.c_str(), req.size());
	
	return TRUE;
}

int NewInitCGITask::Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select)
{
	_error_code = CGI_CODE_LOGIN_FAIL;

	m_res = new NewInitResult;
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

			//newInit作用:初始化同步key,拉取好友信息,刷新个人信息,获取离线期间服务器上的未同步过信息
			com::tencent::mars::microchat::proto::NewInitResponse resp;
			bool bRet = resp.ParsePartialFromArray(RespProtobuf.c_str(), RespProtobuf.size());

			if (bRet)
			{
				//保存最新的同步key
				pAuthInfo->SetSyncKey(resp.sync2());
				
				//type==1:个人信息
				//type==2:好友列表
				//type==5:未读消息
				if (resp.cntlist())
				{
					((NewInitResult *)m_res)->ppNewMsg = new NewMsg *[resp.cntlist()];
					((NewInitResult *)m_res)->ppContanct = new ContactInfo *[resp.cntlist()];
					((NewInitResult *)m_res)->dwSize = resp.cntlist();
					
					for (int i = 0; i < resp.cntlist(); i++)
					{
						if (1 == resp.tag7(i).type())
						{
							//刷新个人信息
							com::tencent::mars::microchat::proto::UserInfo info;
							bool ret = info.ParsePartialFromArray(resp.tag7(i).data().data().c_str(), resp.tag7(i).data().data().size());
							if (ret)
							{
								//暂时不在这里处理,newinit成功后使用getprofile刷新个人信息
							}
							else
							{
								_error_code = CGI_CODE_PARSE_PROTOBUF_ERR;
								return 0;
							}
						}
						else if (2 == resp.tag7(i).type())
						{
							//刷新好友列表
							com::tencent::mars::microchat::proto::ContactInfo info;
							bool ret = info.ParsePartialFromArray(resp.tag7(i).data().data().c_str(), resp.tag7(i).data().data().size());
							if (ret)
							{
								ContactInfo *pInfo = new ContactInfo;
								strcpy_s(pInfo->wxid, info.wxid().id().c_str());
								strcpy_s(pInfo->nickName, info.nickname().name().c_str());
								strcpy_s(pInfo->headicon, info.headicon_small().c_str());
								strcpy_s(pInfo->v1_Name, info.encryptname().c_str());

								((NewInitResult *)m_res)->ppContanct[((NewInitResult *)m_res)->dwContanct++] = pInfo;
							
								//好友信息存入本地数据库
								CString strSql;
								CString strWxid = CA2W(pInfo->wxid);
								CString strNickName = CUTF82W(pInfo->nickName);
								CString strHeadIcon = CA2W(pInfo->headicon);
								CString strV1_Name = CA2W(pInfo->v1_Name);
								strSql.Format(L"insert into contact(wxid,nickname,headicon,v1_name) values('%s','%s','%s','%s');", strWxid, strNickName,strHeadIcon,strV1_Name);
								pMicroChatDb->ExecSQL(CW2UTF8(strSql), NULL, NULL);
							}
							else
							{
								_error_code = CGI_CODE_PARSE_PROTOBUF_ERR;
								return 0;
							}
						}
						else if (5 == resp.tag7(i).type())
						{
							//刷新最新消息
							com::tencent::mars::microchat::proto::NewSyncRespMsg_::NewMsgContent_ msg;
							bool ret = msg.ParsePartialFromArray(resp.tag7(i).data().data().c_str(), resp.tag7(i).data().data().size());
							if (ret)
							{
								NewMsg *pMsg = new NewMsg;
								if (pMsg)
								{
									strcpy_s(pMsg->szFrom, msg.from().id().c_str());
									strcpy_s(pMsg->szTo, msg.to().id().c_str());
									strcpy_s(pMsg->szContent, Utf82CStringA(msg.rawcontent().content().c_str()));
									pMsg->nType = msg.type();
									pMsg->utc = msg.createtime();
									pMsg->svrid = msg.serverid();

									((NewInitResult *)m_res)->ppNewMsg[((NewInitResult *)m_res)->dwNewMsg++] = pMsg;
								
									//更新本地数据库
									pMicroChatDb->AddMsg(pMsg);
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
							//垃圾消息
						}
					}
				}
				else
				{
					_error_code = CGI_CODE_PARSE_PROTOBUF_ERR;
					return 0;
				}

			}
		}		
	}
	else
	{
		_error_code = CGI_CODE_DECRYPT_ERR;
		return 0;
	}
		

	return 0;
}

std::string NewInitCGITask::MakeReq()
{
	string req;

	com::tencent::mars::microchat::proto::NewInitRequest newinitReq;
	com::tencent::mars::microchat::proto::NewInitRequest_LoginInfo *pInfo = new com::tencent::mars::microchat::proto::NewInitRequest_LoginInfo;
	com::tencent::mars::microchat::proto::NewInitRequest_Tag3 *pTag3 = new com::tencent::mars::microchat::proto::NewInitRequest_Tag3;
	com::tencent::mars::microchat::proto::NewInitRequest_Tag4 *pTag4 = new com::tencent::mars::microchat::proto::NewInitRequest_Tag4;

	pInfo->set_aeskey(pAuthInfo->m_Session.c_str());
	pInfo->set_uin(pAuthInfo->m_uin);
	pInfo->set_guid(pAuthInfo->m_guid_15);
	pInfo->set_clientver(pAuthInfo->m_ClientVersion);
	pInfo->set_androidver(pAuthInfo->m_androidVer);
	pInfo->set_unknown3(3);

	newinitReq.set_allocated_login(pInfo);
	newinitReq.set_wxid(pAuthInfo->m_WxId);
	newinitReq.set_allocated_tag3(pTag3);
	newinitReq.set_allocated_tag4(pTag4);
	newinitReq.set_language(pAuthInfo->m_launguage);

	newinitReq.SerializeToString(&req);

	newinitReq.release_login();
	newinitReq.release_tag3();
	newinitReq.release_tag4();
	delete pInfo;
	delete pTag3;
	delete pTag4;

	return req;
}
