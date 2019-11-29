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

#include "../XmsgOssCfg.h"
#include "XmsgOssTransmissionMgr.h"

XmsgOssTransmissionMgr* XmsgOssTransmissionMgr::inst = new XmsgOssTransmissionMgr();

XmsgOssTransmissionMgr::XmsgOssTransmissionMgr()
{

}

XmsgOssTransmissionMgr* XmsgOssTransmissionMgr::instance()
{
	return XmsgOssTransmissionMgr::inst;
}

void XmsgOssTransmissionMgr::init()
{
	for (uint i = 0; i < XmsgOssCfg::instance()->cfgPb->misc().objdownloadthread(); ++i)
	{
		string name;
		SPRINTF_STRING(&name, "download-%02X", i)
		shared_ptr<ActorBlockingSingleThread> abst(new ActorBlockingSingleThread(name));
		this->downloadThreads.push_back(abst);
	}
}

void XmsgOssTransmissionMgr::addToken(const string& token, const shared_ptr<XmsgImHlrUsrAuthInfoQueryRsp> authInfo)
{
	unique_lock<mutex> lock(this->lock4tokenCached);
	this->tokenCached[token] = authInfo;
}

const shared_ptr<XmsgImHlrUsrAuthInfoQueryRsp> XmsgOssTransmissionMgr::findToken(const string& token)
{
	unique_lock<mutex> lock(this->lock4tokenCached);
	auto it = this->tokenCached.find(token);
	if (it == this->tokenCached.end())
		return nullptr;
	if (DateMisc::gotoGmt0(Xsc::clock) > it->second->expired()) 
	{
		this->tokenCached.erase(it);
		return nullptr;
	}
	return it->second;
}

void XmsgOssTransmissionMgr::addMsgStub(const string& msg, void* cb)
{
	this->msgStub[msg] = cb;
}

void* XmsgOssTransmissionMgr::getMsgStub(const string& msg)
{
	auto it = this->msgStub.find(msg);
	return it == this->msgStub.end() ? NULL : it->second;
}

void XmsgOssTransmissionMgr::future(function<void()> cb, const string& oid)
{
	int indx = ((oid.data()[oid.length() - 1]) % this->downloadThreads.size()) & 0x00FF;
	this->downloadThreads[indx]->future(cb);
}

XmsgOssTransmissionMgr::~XmsgOssTransmissionMgr()
{

}

