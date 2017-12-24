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
*  CGITask.h
*
*  Created on: 2017-7-7
*      Author: chenzihao
*/

#ifndef _CGI_TASK_H_
#define _CGI_TASK_H_
#include <map>
#include <string>

#include "Business/define.h"
#include "mars/comm/autobuffer.h"

class CGITask
{
public:
	virtual ~CGITask() {};
	virtual bool Req2Buf(uint32_t _taskid, void* const _user_context, AutoBuffer& _outbuffer, AutoBuffer& _extend, int& _error_code, const int _channel_select) = 0;
	virtual int Buf2Resp(uint32_t _taskid, void* const _user_context, const AutoBuffer& _inbuffer, const AutoBuffer& _extend, int& _error_code, const int _channel_select) = 0;

	uint32_t taskid_;
	ChannelType channel_select_;
	uint32_t cmdid_;				//长链接包头使用(短链接不需要)
	uint32_t cgitype_;				//封包body包头使用(长短链接都需要)
	std::string cgi_;				//cgi uri地址
	std::string host_;				//短链接域名

	void *m_res;					//返回给上层回调函数参数;由上层调用SafeFree接口释放内存,sdk只负责申请,不主动释放
};

#endif