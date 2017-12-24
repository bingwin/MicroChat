#include "stdafx.h"
#include "ManualAuthCGITask.h"
#include <mars/comm/windows/projdef.h>
#include "proto/generate/manualauth.pb.h"
#include "business/crypto/mmCryptInterface.h"
#include "AuthInfo.h"
#include "Business/define.h"
#include "Wrapper/NetworkService.h"


bool ManualAuthCGITask::Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select)
{
	GenPwd();
	GenAesKey();
	GenEcdh();

	string reqAccountProtobuf = MakeAccountReq();
	string keye = Hex2bin(LOGIN_RSA_VER158_KEY_E);
	string keyn = Hex2bin(LOGIN_RSA_VER158_KEY_N);
	DWORD dwCompressedAccount = 0;
	string reqAccount = compress_rsa(keye,keyn, reqAccountProtobuf, dwCompressedAccount);
	if (!reqAccount.size())	return FALSE;


	string reqDeviceProtobuf = MakeDeviceReq("", "", "", "", "", "", "","", FALSE);
	DWORD dwCompressedDevice = 0;
	string reqDevice = compress_aes(m_strAesKey, reqDeviceProtobuf, dwCompressedDevice);
	if (!reqDevice.size())	return FALSE;

	string subHeader;
	DWORD dwLenAccountProtobuf = htonl(reqAccountProtobuf.size());
	subHeader = subHeader + string((const char *)&dwLenAccountProtobuf, 4);
	DWORD dwLenDeviceProtobuf = htonl(reqDeviceProtobuf.size());
	subHeader = subHeader + string((const char *)&dwLenDeviceProtobuf, 4);
	DWORD dwLenAccountRsa = htonl(reqAccount.size());
	subHeader = subHeader + string((const char *)&dwLenAccountRsa, 4);

	string body = subHeader + reqAccount + reqDevice;

	string header = MakeHeader(cgitype_, body.size(), body.size());

	string req = header + body;


	_outbuffer.AllocWrite(req.size());
	_outbuffer.Write(req.c_str(),req.size());

	LOG("登录包组包成功,包长%d\r\n等待服务器返回登陆结果......\r\n", req.size());

	return TRUE;
}

int ManualAuthCGITask::Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select)
{
	_error_code = CGI_CODE_LOGIN_FAIL;

	m_res = new LoginResult;
	NEW_ERR(m_res);
	
	LOG("收包成功，包长%d\r\n开始解包......\r\n", _inbuffer.Length());

	string body = UnPackHeader(string((const char *)_inbuffer.Ptr(), _inbuffer.Length()));

	if (!body.size())
	{
		LOG("封包异常，请按mm协议正确发送请求!\r\n", _inbuffer.Length());

		//包头解包失败
		_error_code = CGI_CODE_UNPACK_ERR;
		return 0;
	}

	string RespProtobuf;
	if (m_bCompressed)
		RespProtobuf = aes_uncompress(m_strAesKey, body, m_nLenRespProtobuf);
	else
		RespProtobuf = aes_nouncompress(m_strAesKey, body);


	com::tencent::mars::sample::proto::ManualAuthResponse resp;
	bool bRet = resp.ParsePartialFromArray(RespProtobuf.c_str(), RespProtobuf.size());

	if (bRet)
	{
		LOG("ManualAuth解包成功,protobuf长度%d\r\n", RespProtobuf.size());
		
		//保存本次登陆code和结果msg
		int nCode = resp.result().code();
		_error_code = nCode;	
		strcpy_s(((LoginResult *)m_res)->szMsg, resp.result().err_msg().msg().c_str());

		CStringA authMsg = Utf82CStringA(resp.result().err_msg().msg().c_str());
		LOG("本次登陆服务器返回code:%d,err_msg:\r\n%s\r\n", nCode, authMsg);

		if (!nCode)
		{
			if (resp.authparam().ecdh().ecdhkey().len())
			{
				string strECServrPubKey = resp.authparam().ecdh().ecdhkey().key();

				//ECDH握手
				string aesKey = DoEcdh(m_nid, strECServrPubKey, m_strLocalEcdhPriKey);

				if (resp.authparam().session().len())
				{
					string strSessionKey = resp.authparam().session().key();

					string session = aes_nouncompress(aesKey, strSessionKey);

					if (session.size())
					{
						LOG("\r\n===============登录成功===============\r\n");
						LOG("ECDH握手成功，本次登录session:%s\r\n", session.c_str());
						
						//保存登录结果,方便后续通讯使用
						pAuthInfo->m_Session = session.c_str();
						pAuthInfo->m_ClientVersion = s_dwVersion;
						pAuthInfo->m_uin = s_dwUin;
						pAuthInfo->m_UserName = m_strUserName.c_str();
						pAuthInfo->m_WxId = resp.accountinfo().wxid().c_str();
						pAuthInfo->m_guid = DEVICE_INFO_GUID;
						pAuthInfo->m_guid_15 = pAuthInfo->m_guid;
						pAuthInfo->m_guid_15.resize(15);
						pAuthInfo->m_androidVer = DEVICE_INFO_ANDROID_VER;
						pAuthInfo->m_launguage = DEVICE_INFO_LANGUAGE;

						//将wxid和uin返回给上层
						strcpy_s(((LoginResult *)m_res)->szWxid, pAuthInfo->m_WxId.c_str());
						((LoginResult *)m_res)->dwUin = pAuthInfo->m_uin;
						
						//登录成功
						_error_code = CGI_CODE_LOGIN_SUCC;
						return 0;
					}				
				}
				
				LOG("Server sessionKey 解密失败!!!请更新mm协议!\r\n");
			}
			
			LOG("ECDH握手失败!!!请更新mm协议!\r\n");
			_error_code = CGI_CODE_LOGIN_ECDH_ERR;
			return 0;
		}
		else
		{
			//登陆失败

			//解析失败原因(解析失败就返回给上层raw数据)
			string strXml = resp.result().err_msg().msg();
			if (strXml.npos != strXml.find("<Content><![CDATA["))
			{
				int nStart = strXml.find("<Content><![CDATA[") + strlen("<Content><![CDATA[");
				int nEnd = strXml.find("]></Content>", nStart);

				if (strXml.npos != nEnd)
				{
					string strReason = Utf82CStringA(strXml.substr(nStart, nEnd - nStart - 1).c_str());
					strcpy_s(((LoginResult *)m_res)->szMsg, strReason.c_str());
				}
			}

			//需要扫码授权
			if (CGI_CODE_LOGIN_NEED_SCAN_QRCODE == nCode)
			{
				string strXml = resp.result().err_msg().msg();

				if (strXml.npos != strXml.find("<Url><![CDATA["))
				{
					int nStart = strXml.find("<Url><![CDATA[") + strlen("<Url><![CDATA[");
					int nEnd = strXml.find("]]></Url>",nStart);

					if (strXml.npos != nEnd)
					{
						string url = strXml.substr(nStart, nEnd-nStart);
						LOG("您第一次在该设备登录，即将跳转到验证页面,请扫码授权后重新登录!\r\n验证页面地址:%s\r\n",url);

						_error_code = CGI_CODE_LOGIN_NEED_SCAN_QRCODE;
						strcpy_s(((LoginResult *)m_res)->szUrl, url.c_str());
						return 0;
					}
				}
				
				LOG("\r\n二维码页面地址解析失败,请更新mm协议!\r\n");
				_error_code = CGI_CODE_LOGIN_SCAN_QRCODE_ERR;
				return 0;
			}
			else if (CGI_CODE_LOGIN_NEED_MOBILE_MSG == nCode)
			{
				LOG("即将进入短信授权验证验证流程......\r\n");

				//保存获取短信验证码凭据
				pAuthInfo->m_mobilecode_authticket = resp.authparam().l();
				pAuthInfo->m_mobileNum = m_strUserName;

				_error_code = CGI_CODE_LOGIN_NEED_MOBILE_MSG;
				return 0;				
			}
			else
			{
				LOG("\r\n登录失败!\r\n");

				//其他原因导致登录失败
				_error_code = CGI_CODE_LOGIN_FAIL;
				return 0;
			}			
		}
	}
	else
	{		
		LOG("ManualAuth解包失败，请更新mm协议!\r\n");	
		//封包解密失败
		_error_code = CGI_CODE_DECRYPT_ERR;
		return 0;
	}

	return 0;
}

std::string ManualAuthCGITask::MakeHeader(int cgiType, int nLenProtobuf, int nLenCompressed)
{
	string strHeader;

	int nCur = 0;

	//登录包不需要添加移动设备封包标志
	//strHeader.push_back(0xbf);
	//nCur++;

	//登录包包体由三部分组成,不能直接使用压缩算法(最后2bits设为2)
	unsigned char SecondByte = 0x2;

	//包头长度最后写入
	strHeader.push_back(SecondByte);
	nCur++;

	//加密算法(前4bits),RSA加密(7)
	unsigned char ThirdByte = 0x7 << 4;

	//cookie长度(后4bits)，当前协议默认15位
	ThirdByte += 0xf;

	strHeader.push_back(ThirdByte);
	nCur++;

	DWORD dwVer = htonl(s_dwVersion);
	strHeader = strHeader + string((const char *)&dwVer, 4);
	nCur += 4;

	//登录包不需要uin 全0占位即可
	DWORD dwUin = 0;
	strHeader = strHeader + string((const char *)&dwUin, 4);
	nCur += 4;

	//登录包不需要cookie 全0占位即可
	char szCookie[15] = { 0 };
	strHeader = strHeader + string((const char *)szCookie, 15);
	nCur += 15;

	string strCgi = Dword2String(cgiType);
	strHeader = strHeader + strCgi;
	nCur += strCgi.size();

	string strLenProtobuf = Dword2String(nLenProtobuf);
	strHeader = strHeader + strLenProtobuf;
	nCur += strLenProtobuf.size();

	string strLenCompressed = Dword2String(nLenCompressed);
	strHeader = strHeader + strLenCompressed;
	nCur += strLenCompressed.size();

	byte rsaVer = LOGIN_RSA_VER;
	strHeader = strHeader + string((const char *)&rsaVer, 1);
	nCur++;

	byte unkwnow[2] = { 0x01,0x02 };
	strHeader = strHeader + string((const char *)unkwnow, 2);
	nCur += 2;

	//将包头长度写入第二字节前6bits(包头长度不会超出6bits)
	SecondByte += (nCur << 2);

	//将正确的第二字节写入包头
	strHeader[0] = SecondByte;

	return strHeader;
}

std::string ManualAuthCGITask::UnPackHeader(string pack)
{
	string body;
	if (pack.size() < 0x20)	return body;

	int nCur = 0;

	//跳过安卓标志bf(计入包头总长度)
	if (0xbf == (unsigned char)pack[nCur])
	{
		nCur++;
	}

	//解析包头长度(前6bits)
	int nHeadLen = (unsigned char)pack[nCur] >> 2;

	//是否使用压缩(后2bits)
	m_bCompressed = (1 == ((unsigned char)pack[nCur] & 0x3)) ? TRUE : FALSE;

	nCur++;

	//解密算法(前4 bits)(05:aes / 07:rsa)(仅握手阶段的发包使用rsa公钥加密,由于没有私钥收包一律aes解密)
	m_nDecryptType = (unsigned char)pack[nCur] >> 4;

	//cookie长度(后4 bits)
	int nCookieLen = (unsigned char)pack[nCur] & 0xF;

	nCur++;

	//服务器版本,无视(4字节)
	nCur += 4;

	//登录包 保存uin
	DWORD dwUin;
	memcpy(&dwUin, &(pack[nCur]), 4);
	s_dwUin = ntohl(dwUin);
	nCur += 4;


	//刷新cookie(超过15字节说明协议头已更新)
	if (nCookieLen && nCookieLen <= 0xf)
	{
		s_cookie = pack.substr(nCur, nCookieLen);
		pAuthInfo->m_cookie = s_cookie;

		nCur += nCookieLen;
	}
	else if (nCookieLen > 0xf)
	{
		return body;
	}

	//cgi type,变长整数,无视
	DWORD dwLen = 0;
	DWORD dwCgiType = String2Dword(pack.substr(nCur, 5), dwLen);
	nCur += dwLen;

	//解压后protobuf长度，变长整数
	m_nLenRespProtobuf = String2Dword(pack.substr(nCur, 5), dwLen);
	nCur += dwLen;

	//压缩后(加密前)的protobuf长度，变长整数
	m_nLenRespCompressed = String2Dword(pack.substr(nCur, 5), dwLen);
	nCur += dwLen;

	//后面数据无视

	//解包完毕,取包体
	if (nHeadLen < pack.size())
	{
		body = pack.substr(nHeadLen);
	}


	return body;
}

void ManualAuthCGITask::GenPwd()
{
	m_strPwd = GetMd5_32(m_strPwd);
}

void ManualAuthCGITask::GenAesKey()
{
	m_strAesKey.resize(16);
	for (int i = 0; i < 16; i++)
	{
		m_strAesKey[i] = rand() % 0xff;
	}
}

void ManualAuthCGITask::GenEcdh()
{
	m_nid = ECDH_NID;
	::GenEcdh(m_nid, m_strLocalEcdhPubKey, m_strLocalEcdhPriKey);
}

std::string ManualAuthCGITask::MakeAccountReq()
{
	string req;
	com::tencent::mars::sample::proto::ManualAuthAccountRequest accountQuest;

	com::tencent::mars::sample::proto::ManualAuthAccountRequest_AesKey *aesKey = new com::tencent::mars::sample::proto::ManualAuthAccountRequest_AesKey;
	com::tencent::mars::sample::proto::ManualAuthAccountRequest_Ecdh   *ecdh = new com::tencent::mars::sample::proto::ManualAuthAccountRequest_Ecdh;
	com::tencent::mars::sample::proto::ManualAuthAccountRequest_Ecdh_EcdhKey *ecdhKey = new com::tencent::mars::sample::proto::ManualAuthAccountRequest_Ecdh_EcdhKey;

	aesKey->set_len(m_nAesKeyLen);
	aesKey->set_key(m_strAesKey);
	ecdhKey->set_len(m_strLocalEcdhPubKey.size());
	ecdhKey->set_key(m_strLocalEcdhPubKey);
	ecdh->set_allocated_ecdhkey(ecdhKey);
	ecdh->set_nid(m_nid);

	accountQuest.set_allocated_aes(aesKey);
	accountQuest.set_allocated_ecdh(ecdh);
	accountQuest.set_username(m_strUserName);
	accountQuest.set_password1(m_strPwd);
	accountQuest.set_password2(m_strPwd);

	accountQuest.SerializeToString(&req);

	accountQuest.release_aes();
	accountQuest.release_ecdh();
	ecdh->release_ecdhkey();
	delete aesKey;
	delete ecdh;
	delete ecdhKey;

	return req;
}

std::string ManualAuthCGITask::MakeDeviceReq(string guid,string androidVer,string imei,string manufacturer,string modelName,string wifiMacAddress, string apBssid, string clientSeqId,bool bRandom)
{
#if 0
	//允许载入二进制设备数据登录
	string data;
	data = Hex2bin("");
	return data;
#else
	//使用指定设备登录
	string req;
	com::tencent::mars::sample::proto::ManualAuthDeviceRequest deviceQuest;

	com::tencent::mars::sample::proto::ManualAuthDeviceRequest_LoginInfo *loginInfo = new com::tencent::mars::sample::proto::ManualAuthDeviceRequest_LoginInfo;
	com::tencent::mars::sample::proto::ManualAuthDeviceRequest_UnknowInfo2  *unknownInfo2 = new com::tencent::mars::sample::proto::ManualAuthDeviceRequest_UnknowInfo2;
	com::tencent::mars::sample::proto::ManualAuthDeviceRequest_UnknowInfo2_Tag1 *tag1 = new com::tencent::mars::sample::proto::ManualAuthDeviceRequest_UnknowInfo2_Tag1;
	com::tencent::mars::sample::proto::ManualAuthDeviceRequest_UnknowInfo2_Tag2 *tag2 = new com::tencent::mars::sample::proto::ManualAuthDeviceRequest_UnknowInfo2_Tag2;
	com::tencent::mars::sample::proto::ManualAuthDeviceRequest_UnknowInfo2_Tag2_Tag4 *tag2_tag4 = new com::tencent::mars::sample::proto::ManualAuthDeviceRequest_UnknowInfo2_Tag2_Tag4;
	tag2->set_allocated_tag4(tag2_tag4);
	com::tencent::mars::sample::proto::ManualAuthDeviceRequest_UnknowInfo2_Tag3 *tag3 = new com::tencent::mars::sample::proto::ManualAuthDeviceRequest_UnknowInfo2_Tag3;
	com::tencent::mars::sample::proto::ManualAuthDeviceRequest_UnknowInfo2_Tag4 *tag4 = new com::tencent::mars::sample::proto::ManualAuthDeviceRequest_UnknowInfo2_Tag4;
	com::tencent::mars::sample::proto::ManualAuthDeviceRequest_UnknowInfo2_Tag5 *tag5 = new com::tencent::mars::sample::proto::ManualAuthDeviceRequest_UnknowInfo2_Tag5;
	unknownInfo2->set_allocated_tag1(tag1);
	unknownInfo2->set_allocated_tag2(tag2);
	unknownInfo2->set_allocated_tag3(tag3);
	unknownInfo2->set_allocated_tag4(tag4);
	unknownInfo2->set_allocated_tag5(tag5);
	unknownInfo2->set_tag6(0);

	loginInfo->set_unknown1(string(""));
	loginInfo->set_unknown2(0);
	string guid15 = string(DEVICE_INFO_GUID, 15);
	loginInfo->set_guid(string(guid15.c_str(), 16));
	loginInfo->set_clientver(s_dwVersion);
	loginInfo->set_androidver(DEVICE_INFO_ANDROID_VER);
	loginInfo->set_unknown3(1);
	
	CStringA strSoftInfo;
	strSoftInfo.Format(DEVICE_INFO_SOFTINFO, DEVICE_INFO_IMEI,DEVICE_INFO_ANDROID_ID, DEVICE_INFO_MANUFACTURER+" "+DEVICE_INFO_MODELNAME, DEVICE_INFO_MOBILE_WIFI_MAC_ADDRESS, DEVICE_INFO_CLIENT_SEQID_SIGN, DEVICE_INFO_AP_BSSID, DEVICE_INFO_MANUFACTURER,"taurus", DEVICE_INFO_MODELNAME, DEVICE_INFO_IMEI);

	CStringA strDeviceInfo;
	strDeviceInfo.Format(DEVICE_INFO_DEVICEINFO, DEVICE_INFO_MANUFACTURER, DEVICE_INFO_MODELNAME);

	deviceQuest.set_allocated_login(loginInfo);
	deviceQuest.set_allocated_unknown2(unknownInfo2);
	deviceQuest.set_imei(DEVICE_INFO_IMEI);
	deviceQuest.set_softinfoxml(strSoftInfo);
	deviceQuest.set_unknown5(0);
	deviceQuest.set_clientseqid(DEVICE_INFO_CLIENT_SEQID);
	deviceQuest.set_clientseqid_sign(DEVICE_INFO_CLIENT_SEQID_SIGN);
	deviceQuest.set_logindevicename(DEVICE_INFO_MANUFACTURER+" "+DEVICE_INFO_MODELNAME);
	deviceQuest.set_deviceinfoxml(strDeviceInfo);
	deviceQuest.set_language(DEVICE_INFO_LANGUAGE);
	deviceQuest.set_timezone("8.00");
	deviceQuest.set_unknown13(0);
	deviceQuest.set_unknown14(0);
	deviceQuest.set_devicebrand(DEVICE_INFO_MANUFACTURER);
	deviceQuest.set_devicemodel(DEVICE_INFO_MODELNAME+"armeabi-v7a");
	deviceQuest.set_ostype(DEVICE_INFO_ANDROID_VER);
	deviceQuest.set_realcountry("cn");
	deviceQuest.set_unknown22(2);

	deviceQuest.SerializeToString(&req);

	unknownInfo2->release_tag1();
	unknownInfo2->release_tag2();
	unknownInfo2->release_tag3();
	unknownInfo2->release_tag4();
	unknownInfo2->release_tag5();
	tag2->release_tag4();
	deviceQuest.release_unknown2();
	deviceQuest.release_login();

	delete tag1;
	delete tag2;
	delete tag3;
	delete tag4;
	delete tag5;
	delete tag2_tag4;
	delete unknownInfo2;
	delete loginInfo;


	return req;
#endif
}

