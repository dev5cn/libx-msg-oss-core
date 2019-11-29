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

#include "XmsgOssIpfsOper.h"
#include "../XmsgOssCfg.h"

XmsgOssIpfsOper* XmsgOssIpfsOper::inst = new XmsgOssIpfsOper();

XmsgOssIpfsOper::XmsgOssIpfsOper()
{

}

XmsgOssIpfsOper* XmsgOssIpfsOper::instance()
{
	return XmsgOssIpfsOper::inst;
}

bool XmsgOssIpfsOper::init()
{
	for (uint i = 0; i < XmsgOssCfg::instance()->cfgPb->misc().ipfsuploadthread(); ++i) 
	{
		string name;
		SPRINTF_STRING(&name, "ipfs-u-%02X", i)
		this->abstu.push_back(shared_ptr<ActorBlockingSingleThread>(new ActorBlockingSingleThread(name)));
	}
	for (uint i = 0; i < XmsgOssCfg::instance()->cfgPb->misc().ipfsdownloadthread(); ++i) 
	{
		string name;
		SPRINTF_STRING(&name, "ipfs-d-%02X", i)
		this->abstd.push_back(shared_ptr<ActorBlockingSingleThread>(new ActorBlockingSingleThread(name)));
	}
	return true;
}

void XmsgOssIpfsOper::futureu(function<void()> cb)
{
	this->abstu.at(Crypto::randomInt() % this->abstu.size())->future(cb);
}

void XmsgOssIpfsOper::futured(function<void()> cb)
{
	this->abstd.at(Crypto::randomInt() % this->abstd.size())->future(cb);
}

XmsgOssIpfsOper::~XmsgOssIpfsOper()
{

}

