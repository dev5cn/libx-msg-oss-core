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

#ifndef CHANNEL_XMSGOSSHTTPCHANNEL_H_
#define CHANNEL_XMSGOSSHTTPCHANNEL_H_

#include <libx-msg-im-xsc.h>
#include <libx-msg-oss-pb.h>
#include "../up-down/XmsgOss4download.h"
#include "../up-down/XmsgOss4upload.h"

class XmsgOssHttpChannel: public XscHttpChannel
{
public:
	shared_ptr<XmsgClientTokenInfo> token; 
	shared_ptr<XmsgImHlrUsrAuthInfoQueryRsp> authInfo; 
	shared_ptr<Message> msg; 
	SptrCgt cgt; 
public:
	bool evnHeader(XscHttpWorker* wk, map<string, string>& header); 
	bool evnBody(XscHttpWorker* wk, uchar* dat, int len, bool more); 
	void evnCanSend(); 
	void evnDisc(); 
	void dida(ullong now); 
	void clean(); 
public:
	bool sendRet(int ret, const string& desc); 
	bool sendXmsgOssUploadSimpleRsp(shared_ptr<XmsgOssUploadSimpleRsp> rsp); 
	string toString();
	XmsgOssHttpChannel(XscHttpWorker* wk, int mtu, int cfd, const string &peer);
	virtual ~XmsgOssHttpChannel();
public:
	shared_ptr<XmsgOss4upload> upload; 
	vector<uchar> datCache; 
	bool evnHeader4upload(const string& msgName, const string& msgDat); 
	bool evnBody4upload(uchar* dat, int len, bool more); 
	void handleXmsgImAuthRsp4upload(SptrXiti itrans); 
public:
	shared_ptr<XmsgOss4download> download; 
	bool evnHeader4download(const string& msgName, const string& msgDat); 
	void handleXmsgImAuthRsp4download(SptrXiti itrans); 
private:
	void* cb; 
	bool verifySign(shared_ptr<XmsgImHlrUsrAuthInfoQueryRsp> authInfo); 
};

#endif 
