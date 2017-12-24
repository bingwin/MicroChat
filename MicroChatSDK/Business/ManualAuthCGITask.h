#pragma once

#include "Wrapper/CGITask.h"
#include "mars/boost/weak_ptr.hpp"
#include "fun.h"
#include "MakeHeader.h"


class ManualAuthCGITask;


class ManualAuthCGITask : public CGITask,public BaseHeader
{
public:
	virtual bool Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select);
	virtual int Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select);

	string		m_strUserName;
	string		m_strPwd;
private:
	void GenPwd();				//密码Md5
	void GenAesKey();			//握手包aes(仅用于本次通讯,后续通讯使用ECDH协商出的session key)
	void GenEcdh();				//生成本地DH
	string MakeAccountReq();
	string MakeDeviceReq(string guid, string androidVer, string imei, string manufacturer, string modelName, string wifiMacAddress, string apBssid, string clientSeqId, bool bRandom = FALSE);
	
	virtual string MakeHeader(int cgiType, int nLenProtobuf, int nLenCompressed);
	virtual string UnPackHeader(string pack);

	string		m_strAesKey;
	int			m_nAesKeyLen = 16;
	string		m_strLocalEcdhPubKey;
	string		m_strLocalEcdhPriKey;
	int			m_nid = ECDH_NID;
};