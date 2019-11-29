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
#include "XmsgOss4upload.h"

XmsgOss4upload::XmsgOss4upload()
{
	this->req = nullptr;
	this->objSize = 0;
	this->storeSize = 0;
	this->fd = -1;
	this->sc = NULL;
	this->info.reset(new XmsgKv());
	this->finished = false;
}

void XmsgOss4upload::evnDisc()
{
	if (this->finished) 
		return;
	this->finished = true;
	if (this->storeSize > 0) 
		::remove((this->storePath + this->oid).c_str()); 
	if (this->fd != -1)
		::close(this->fd);
	this->fd = -1;
}

void XmsgOss4upload::finish()
{
	this->finished = true;
	if (this->fd != -1)
		::close(this->fd);
	this->fd = -1;
}

void XmsgOss4upload::calHash(uchar* dat, int len, bool more)
{
	int seg = XmsgOssCfg::instance()->cfgPb->misc().objhashsegment();
	if (this->sc == NULL)
	{
		if (more) 
		{
			this->sc = Crypto::sha256init();
			int loop = len / seg;
			for (int i = 0; i < loop; ++i)
				Crypto::sha256update(this->sc, dat + (i * seg), seg); 
			int offset = loop * seg;
			for (int i = offset; i < len; ++i) 
				this->cached4hash.push_back(dat[i]);
			return;
		}
		this->hashVal = Crypto::sha256ToHexStrLowerCase(dat, len); 
		return;
	}
	int fill = seg - this->cached4hash.size();
	for (int i = 0; i < fill && i < len; ++i) 
		this->cached4hash.push_back(dat[i]);
	if (more)
	{
		if (len < fill) 
			return;
		Crypto::sha256update(this->sc, this->cached4hash.data(), seg); 
		this->cached4hash.clear();
		if (len == fill) 
			return;
		int loop = (len - fill) / seg;
		for (int i = 0; i < loop; ++i)
			Crypto::sha256update(this->sc, dat + fill + (i * seg), seg); 
		int offset = loop * seg + fill;
		for (int i = offset; i < len; ++i) 
			this->cached4hash.push_back(dat[i]);
		return;
	}
	Crypto::sha256update(this->sc, this->cached4hash.data(), this->cached4hash.size()); 
	this->cached4hash.clear();
	if (len < fill || len == fill) 
	{
		this->hashVal = Crypto::sha256final2hexStrLowerCase(this->sc);
		this->sc = NULL;
		return;
	}
	int loop = (len - fill) / seg;
	for (int i = 0; i < loop; ++i)
		Crypto::sha256update(this->sc, dat + fill + (i * seg), seg); 
	int offset = loop * seg + fill;
	Crypto::sha256update(this->sc, dat + offset, len - offset); 
	this->hashVal = Crypto::sha256final2hexStrLowerCase(this->sc);
	this->sc = NULL;
}

string XmsgOss4upload::toString()
{
	string str;
	SPRINTF_STRING(&str, "storageType: %d, req: %s, cgt: %s, hashVal: %s, obj: %s%s, objSize: %llu, storeSize: %llu, info: %s, fd: %d, finished: %s", 
			this->storageType,
			this->req == nullptr ? "null" : this->req->ShortDebugString().c_str(),
			this->cgt == nullptr ? "null" : this->cgt->toString().c_str(),
			this->hashVal.c_str(),
			this->storePath.c_str(),
			this->oid.c_str(),
			this->objSize,
			this->storeSize,
			this->info->ShortDebugString().c_str(),
			this->fd,
			this->finished ? "true" : "false")
	return str;
}

XmsgOss4upload::~XmsgOss4upload()
{
	if (this->sc != NULL)
		::free(sc);
	this->sc = NULL;
}

