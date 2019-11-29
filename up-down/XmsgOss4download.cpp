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
#include "XmsgOss4download.h"
#include "../channel/XmsgOssHttpChannel.h"

XmsgOss4download::XmsgOss4download()
{
	this->fd = 0;
	this->writeSize = 0;
	this->len = 0;
}

void XmsgOss4download::evnSend(shared_ptr<XscChannel> channel)
{
	if (this->fd < 1) 
	{
		LOG_DEBUG("file descriptor not set or download task finished, channel: %s", channel->toString().c_str())
		return;
	}
	ullong remain = this->len - this->writeSize; 
	int seg = (int) XmsgOssCfg::instance()->cfgPb->misc().objdownloadwritebufsize();
	if (remain > (ullong) seg)
	{
		uchar* buf = (uchar*) ::malloc(seg);
		int rlen = Misc::readAll(this->fd, buf, seg);
		if (rlen != seg)
		{
			LOG_FAULT("it`s a bug, read object length not equals buffer length, rlen: %d, len: %d, errno: %s, channel: %s", rlen, seg, ::strerror(errno), channel->toString().c_str())
			::close(this->fd);
			this->fd = 0;
			channel->close(); 
			return;
		}
		this->writeSize += seg;
		channel->send(buf, seg); 
		::free(buf);
		return;
	}
	uchar* buf = (uchar*) ::malloc(remain);
	int rlen = Misc::readAll(this->fd, buf, (int) remain);
	if (rlen != (int) remain)
	{
		LOG_FAULT("it`s a bug, read object length not equals buffer length, rlen: %d, len: %d, errno: %s, channel: %s", rlen, (int )remain, ::strerror(errno), channel->toString().c_str())
		::free(buf);
		::close(this->fd);
		this->fd = 0;
		channel->close(); 
		return;
	}
	::close(this->fd);
	this->fd = 0;
	if (channel->proType == XscProtocolType::XSC_PROTOCOL_HTTP)
	{
		shared_ptr<XmsgOssHttpChannel> httpChannel = static_pointer_cast<XmsgOssHttpChannel>(channel);
		this->writeSize += remain;
		LOG_DEBUG("have a object download request handle finished, len: %llu, write-len: %llu, channel: %s", this->len, this->writeSize, channel->toString().c_str())
		httpChannel->sendBinNoMore(buf, remain);
		::free(buf);
		return;
	}
	::free(buf);
	LOG_FAULT("it`s a bug, only support XSC_PROTOCOL_HTTP, channel: %02X", channel->proType)
}

void XmsgOss4download::evnDisc()
{

}

XmsgOss4download::~XmsgOss4download()
{
	if (this->fd != 0)
		::close(this->fd);
	this->fd = 0;
}

