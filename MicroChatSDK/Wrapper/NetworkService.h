// Tencent is pleased to support the open source community by making Mars available.
// Copyright (C) 2017 THL A29 Limited, a Tencent company. All rights reserved.

// Licensed under the MIT License (the "License"); you may not use this file except in 
// compliance with the License. You may obtain a copy of the License at
// http://opensource.org/licenses/MIT

// Unless required by applicable law or agreed to in writing, software distributed under the License is
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
// either express or implied. See the License for the specific language governing permissions and
// limitations under the License.

/*
*  NetworkService.h
*
*  Created on: 2017-7-7
*      Author: chenzihao
*/

#ifndef _MARS_SERVICE_PROXY_H_
#define _MARS_SERVICE_PROXY_H_
#include <queue>
#include <map>
#include <string>

#include "mars/comm/thread/thread.h"
#include "mars/comm/autobuffer.h"
#include "Wrapper/CGITask.h"
#include "NetworkObserver.h"
#include <minwinbase.h>
#include "interface.h"

class NetworkService : public PushObserver
{
public:
	static NetworkService& Instance();
	bool Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select);
	int Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select);
	int OnTaskEnd(uint32_t _taskid, void* const _user_context, int _error_type, int _error_code);
	virtual void OnPush(uint64_t _channel_id, uint32_t _cmdid, uint32_t _taskid, const AutoBuffer& _body, const AutoBuffer& _extend);
	
	//长链接确认
	//只有确认后服务器才会直接推送cmdid==122的消息内容到客户端;
	//否则服务器只下发cmdid==24的消息通知客户端有新消息,消息内容需要客户端主动调用newsync获取
	int  GetLonglinkIdentifyCheckBuffer(AutoBuffer& _identify_buffer, AutoBuffer& _buffer_hash, int32_t& _cmdid);
	bool OnLonglinkIdentifyResponse(const AutoBuffer& _response_buffer, const AutoBuffer& _identify_buffer_hash);

	//同步消息
	void RequestSync();

	void setClientVersion(uint32_t _client_version);
	void setShortLinkDebugIP(const std::string& _ip, unsigned short _port);
	void setShortLinkPort(unsigned short _port);
	void setLongLinkAddress(const std::string& _ip, vector<uint16_t> &ports, const std::string& _debug_ip = "");
	void setCgiCallBack(CGICallBack callback);
	void DoCallBack(void *result, int nCgiType, int nTaskId, int nCode);
	void start();
	void stop();

	int startTask(CGITask* task,bool bSendOnly = FALSE);

protected:
	NetworkService();
	~NetworkService();

	void __Init();
	void __Destroy();
private:
	std::map<uint32_t, CGITask*> map_task_;

	CRITICAL_SECTION	m_cs;
	CGICallBack			m_cgiCallBack = NULL;
	string				m_shortlinkHost;
};
#define pNetworkService (NetworkService::Instance())

#endif