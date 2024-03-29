/*
  Copyright 2019 www.dev5.cn, Inc. dev5@qq.com
 
  This file is part of X-MSG-IM.
 
  X-MSG-IM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  X-MSG-IM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU Affero General Public License
  along with X-MSG-IM.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef UP_DOWN_XMSGOSSTRANSMISSIONMGR_H_
#define UP_DOWN_XMSGOSSTRANSMISSIONMGR_H_

#include <libx-msg-common-dat-struct-cpp.h>
#include <libx-msg-oss-pb.h>

class XmsgOssTransmissionMgr
{
public:
	void init(); 
	void addToken(const string& token, const shared_ptr<XmsgImHlrUsrAuthInfoQueryRsp> authInfo); 
	const shared_ptr<XmsgImHlrUsrAuthInfoQueryRsp> findToken(const string& token); 
	void future(function<void()> cb, const string& oid); 
public:
	void addMsgStub(const string& msg, void* cb);
	void* getMsgStub(const string& msg);
	static XmsgOssTransmissionMgr* instance();
private:
	unordered_map<string, shared_ptr<XmsgImHlrUsrAuthInfoQueryRsp>> tokenCached; 
	unordered_map<string, void*> msgStub; 
	vector<shared_ptr<ActorBlockingSingleThread>> downloadThreads; 
	mutex lock4tokenCached; 
	static XmsgOssTransmissionMgr* inst;
	XmsgOssTransmissionMgr();
	virtual ~XmsgOssTransmissionMgr();
};

#endif 
