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

#include "XmsgOssHttpChannel.h"
#include "../XmsgOssCfg.h"
#include "../up-down/XmsgOssTransmissionMgr.h"
#include "../usr/XmsgClient.h"

XmsgOssHttpChannel::XmsgOssHttpChannel(XscHttpWorker* wk, int mtu, int cfd, const string &peer) :
		XscHttpChannel(wk, mtu, cfd, peer)
{
	this->cb = NULL;
}

bool XmsgOssHttpChannel::evnHeader(XscHttpWorker* wk, map<string, string>& header)
{
	if (Log::isRecord())
	{
		string str;
		SPRINTF_STRING(&str, "  <-- PEER: %s METHOD: %s PATH: %s VER: %s\nheader:", this->peer.c_str(), this->method.c_str(), this->path.c_str(), this->ver.c_str())
		for (auto& it : header)
		{
			SPRINTF_STRING(&str, "%s: %s\n", it.first.c_str(), it.second.c_str())
		}
		LOG_RECORD("\n%s", str.c_str())
	}
	string tokenInfo = this->getHeader("x-msg-client-token");
	if (tokenInfo.empty())
	{
		LOG_DEBUG("missing required http-header: 'x-msg-client-token', this: %s", this->toString().c_str())
		return this->sendRet(RET_FORMAT_ERROR, "missing required http-header: x-msg-client-token");
	}
	tokenInfo = Crypto::base64dec(tokenInfo);
	Message* msg = XmsgImMsgStub::newPbMsg(XmsgClientTokenInfo::descriptor(), (uchar*) tokenInfo.data(), tokenInfo.length());
	if (msg == nullptr)
	{
		LOG_DEBUG("can not reflect message from data, msg: %s, this: %s", XmsgClientTokenInfo::descriptor()->name().c_str(), this->toString().c_str())
		return this->sendRet(RET_FORMAT_ERROR, "http-header: x-msg-client-token format error");
	}
	this->token.reset((XmsgClientTokenInfo*) msg);
	if (this->token->cgt().empty() || this->token->token().empty() || this->token->alg().empty() || this->token->slat().empty() || this->token->sign().empty() || this->token->plat().empty() || this->token->did().empty())
	{
		LOG_DEBUG("request token info format error: %s, this: %s", this->token->ShortDebugString().c_str(), this->toString().c_str())
		return this->sendRet(RET_FORMAT_ERROR, "http-header: x-msg-client-token format error");
	}
	this->cgt = ChannelGlobalTitle::parse(this->token->cgt());
	if (cgt == nullptr)
	{
		LOG_DEBUG("request token info format error: %s, this: %s", this->token->ShortDebugString().c_str(), this->toString().c_str())
		return this->sendRet(RET_FORMAT_ERROR, "http-header: x-msg-client-token format error");
	}
	string msgName = this->getHeader("x-msg-name");
	if (msgName.empty())
	{
		LOG_DEBUG("missing required http-header: 'x-msg-name', this: %s", this->toString().c_str())
		return this->sendRet(RET_FORMAT_ERROR, "missing required http-header: x-msg-name");
	}
	string msgDat = this->getHeader("x-msg-dat"); 
	if (!msgDat.empty())
		msgDat = Crypto::base64dec(msgDat);
	this->cb = XmsgOssTransmissionMgr::instance()->getMsgStub(msgName);
	if (this->cb == NULL)
	{
		LOG_DEBUG("can not found message stub for x-msg: %s, this: %s", msgName.c_str(), this->toString().c_str())
		return this->sendRet(RET_FORBIDDEN, "unexpected message name");
	}
	if (XmsgOssUploadSimpleReq::descriptor()->name() == msgName) 
		return this->evnHeader4upload(msgName, msgDat);
	if (XmsgOssDownloadSimpleReq::descriptor()->name() == msgName) 
		return this->evnHeader4download(msgName, msgDat);
	LOG_FAULT("it`s a bug, unexpected message: %s", msgName.c_str())
	return this->sendRet(RET_EXCEPTION, "system exception");
}

bool XmsgOssHttpChannel::evnBody(XscHttpWorker* wk, uchar* dat, int len, bool more)
{
	if (this->msg == nullptr)
	{
		LOG_FAULT("it`s a bug, this->msg is null, this: %s", this->toString().c_str())
		return this->sendRet(RET_EXCEPTION, "system exception");
	}
	if (this->cb == nullptr)
	{
		LOG_FAULT("it`s a bug, call back is null, this: %s", this->toString().c_str())
		return this->sendRet(RET_EXCEPTION, "system exception");
	}
	if (XmsgOssUploadSimpleReq::descriptor()->name() == this->msg->GetDescriptor()->name()) 
		return this->evnBody4upload(dat, len, more);
	LOG_DEBUG("unexpected http-body, x-msg-name: %s, this: %s, dat: %s", this->msg->GetDescriptor()->name().c_str(), this->toString().c_str(), Net::hex2strUperCaseSpace(dat, len).c_str())
	return this->sendRet(RET_FORBIDDEN, "unsupported http-body");
}

void XmsgOssHttpChannel::evnCanSend()
{
	if (this->download != nullptr)
		this->download->evnSend(static_pointer_cast<XscChannel>(this->shared_from_this()));
}

void XmsgOssHttpChannel::evnDisc()
{
	LOG_DEBUG("x-msg-client http channel lost: %s", this->toString().c_str())
	if (this->upload != nullptr)
		this->upload->evnDisc();
	if (this->download != nullptr)
		this->download->evnDisc();
}

void XmsgOssHttpChannel::dida(ullong now)
{

}

void XmsgOssHttpChannel::clean()
{

}

bool XmsgOssHttpChannel::evnHeader4upload(const string& msgName, const string& msgDat)
{
	if (this->expectedBodyLen < 1) 
	{
		LOG_DEBUG("Content-Length must be > 0, this: %s", this->toString().c_str())
		return this->sendRet(RET_FORBIDDEN, "Content-Length format error");
	}
	Message* msg = XmsgImMsgStub::newPbMsg(XmsgOssUploadSimpleReq::descriptor(), (uchar*) msgDat.data(), msgDat.length());
	if (msg == nullptr)
	{
		LOG_DEBUG("can not reflect message from data, msg: %s, this: %s", msgName.c_str(), this->toString().c_str())
		return this->sendRet(RET_FORMAT_ERROR, "message XmsgOssUploadSimpleReq format error");
	}
	this->msg.reset(msg);
	shared_ptr<XmsgOssUploadSimpleReq> fur = static_pointer_cast<XmsgOssUploadSimpleReq>(this->msg);
	if (fur->objname().empty() || fur->objsize() < 1)
	{
		LOG_DEBUG("object upload request format error: %s, this: %s", fur->ShortDebugString().c_str(), this->toString().c_str())
		return this->sendRet(RET_FORMAT_ERROR, "message XmsgOssUploadSimpleReq format error");
	}
	auto authInfo = XmsgOssTransmissionMgr::instance()->findToken(this->token->token());
	if (authInfo != nullptr)
	{
		if (!this->verifySign(authInfo))
		{
			LOG_DEBUG("sign verify failed, req: %s, this: %s", fur->ShortDebugString().c_str(), this->toString().c_str())
			return this->sendRet(RET_FORBIDDEN, "sign error");
		}
		this->authInfo = authInfo;
		return true; 
	}
	auto hlr = XmsgNeMgr::instance()->getHlr();
	if (hlr == nullptr)
	{
		LOG_ERROR("can not allocate x-msg-im-hlr, this: %s", this->toString().c_str())
		return this->sendRet(RET_EXCEPTION, "system exception");
	}
	shared_ptr<XmsgImHlrUsrAuthInfoQueryReq> req(new XmsgImHlrUsrAuthInfoQueryReq());
	req->set_token(this->token->token());
	auto ossHttpChannel = static_pointer_cast<XmsgOssHttpChannel>(this->shared_from_this());
	XmsgImChannel::cast(hlr->channel)->begin(req, [ossHttpChannel, req](SptrXiti itrans) 
	{
		ossHttpChannel->future([ossHttpChannel, itrans] 
				{
					ossHttpChannel->handleXmsgImAuthRsp4upload(itrans);
				});
	});
	return true;
}

void XmsgOssHttpChannel::handleXmsgImAuthRsp4upload(SptrXiti itrans)
{
	if (itrans->ret != RET_SUCCESS)
	{
		LOG_DEBUG("query x-msg-client auth info failed, ret: %04X, desc: %s", itrans->ret, itrans->desc.c_str())
		if (itrans->ret == RET_NOT_FOUND)
		{
			this->sendRet(RET_FORBIDDEN, "login first");
			return;
		}
		this->sendRet(RET_EXCEPTION, "system exception");
		return;
	}
	shared_ptr<XmsgImHlrUsrAuthInfoQueryRsp> authInfo = static_pointer_cast<XmsgImHlrUsrAuthInfoQueryRsp>(itrans->endMsg);
	XmsgOssTransmissionMgr::instance()->addToken(this->token->token(), authInfo); 
	if (!this->verifySign(authInfo))
	{
		LOG_DEBUG("sign verify failed, rsp: %s, this: %s", authInfo->ShortDebugString().c_str(), this->toString().c_str())
		this->sendRet(RET_FORBIDDEN, "sign error");
		return;
	}
	this->authInfo = authInfo;
	this->evnBody4upload(this->datCache.data(), this->datCache.size(), this->datCache.size() != this->expectedBodyLen);
	this->datCache.clear();
}

bool XmsgOssHttpChannel::evnBody4upload(uchar* dat, int len, bool more)
{
	if (this->authInfo == nullptr)
	{
		LOG_DEBUG("x-msg-client identify not confirm, we will cache the upload object data, this: %s", this->toString().c_str())
		for (int i = 0; i < len; ++i) 
			this->datCache.push_back(dat[i]);
		return true;
	}
	auto channel = static_pointer_cast<XscChannel>(this->shared_from_this());
	if (this->upload == nullptr) 
	{
		this->upload.reset(new XmsgOss4upload());
		this->upload->storageType = XmsgOssCfg::instance()->cfgPb->misc().storage();
		this->upload->cgt = this->cgt;
		this->upload->objSize = this->expectedBodyLen; 
	}
	((void (*)(shared_ptr<XscChannel> channel, shared_ptr<XmsgOssUploadSimpleReq> req, shared_ptr<XmsgOss4upload> upload, uchar* dat, int len, bool more)) (this->cb)) 
	(channel, static_pointer_cast<XmsgOssUploadSimpleReq>(this->msg), this->upload, dat, len, more);
	return true;
}

bool XmsgOssHttpChannel::evnHeader4download(const string& msgName, const string& msgDat)
{
	if (this->method != "GET")
	{
		LOG_DEBUG("support GET method only, msg: %s, this: %s", msgName.c_str(), this->toString().c_str())
		return this->sendRet(RET_FORBIDDEN, "support GET method only");
	}
	Message* msg = XmsgImMsgStub::newPbMsg(XmsgOssDownloadSimpleReq::descriptor(), (uchar*) msgDat.data(), msgDat.length());
	if (msg == nullptr)
	{
		LOG_DEBUG("can not reflect message from data, msg: %s, this: %s", msgName.c_str(), this->toString().c_str())
		return this->sendRet(RET_FORMAT_ERROR, "message XmsgOssDownloadReq format error");
	}
	this->msg.reset(msg);
	auto authInfo = XmsgOssTransmissionMgr::instance()->findToken(this->token->token());
	if (authInfo != nullptr)
	{
		if (!this->verifySign(authInfo))
		{
			LOG_DEBUG("sign verify failed, req: %s, this: %s", msg->ShortDebugString().c_str(), this->toString().c_str())
			return this->sendRet(RET_FORBIDDEN, "sign error");
		}
		this->authInfo = authInfo;
		this->download.reset(new XmsgOss4download());
		this->download->cgt = this->cgt;
		auto channel = static_pointer_cast<XscChannel>(this->shared_from_this());
		((void (*)(shared_ptr<XscChannel> channel, shared_ptr<XmsgOssDownloadSimpleReq> req, shared_ptr<XmsgOss4download> download)) (this->cb))(channel, static_pointer_cast<XmsgOssDownloadSimpleReq>(this->msg), this->download);
		return true;
	}
	auto hlr = XmsgNeMgr::instance()->getHlr();
	if (hlr == nullptr)
	{
		LOG_ERROR("can not allocate x-msg-im-hlr, this: %s", this->toString().c_str())
		return this->sendRet(RET_EXCEPTION, "system exception");
	}
	shared_ptr<XmsgImHlrUsrAuthInfoQueryReq> req(new XmsgImHlrUsrAuthInfoQueryReq());
	req->set_token(this->token->token());
	auto ossHttpChannel = static_pointer_cast<XmsgOssHttpChannel>(this->shared_from_this());
	XmsgImChannel::cast(hlr->channel)->begin(req, [ossHttpChannel, req](SptrXiti itrans) 
	{
		ossHttpChannel->future([ossHttpChannel, itrans] 
				{
					ossHttpChannel->handleXmsgImAuthRsp4download(itrans);
				});
	});
	return true;
}

void XmsgOssHttpChannel::handleXmsgImAuthRsp4download(SptrXiti itrans)
{
	if (itrans->ret != RET_SUCCESS)
	{
		LOG_DEBUG("query x-msg-client auth info failed, ret: %04X, desc: %s", itrans->ret, itrans->desc.c_str())
		if (itrans->ret == RET_NOT_FOUND) 
		{
			this->sendRet(RET_FORBIDDEN, "login first");
			return;
		}
		this->sendRet(RET_EXCEPTION, "system exception");
		return;
	}
	shared_ptr<XmsgImHlrUsrAuthInfoQueryRsp> authInfo = static_pointer_cast<XmsgImHlrUsrAuthInfoQueryRsp>(itrans->endMsg);
	XmsgOssTransmissionMgr::instance()->addToken(this->token->token(), authInfo); 
	if (!this->verifySign(authInfo))
	{
		LOG_DEBUG("sign verify failed, rsp: %s, this: %s", authInfo->ShortDebugString().c_str(), this->toString().c_str())
		this->sendRet(RET_FORBIDDEN, "sign error");
		return;
	}
	this->authInfo = authInfo;
	this->download.reset(new XmsgOss4download());
	this->download->cgt = this->cgt;
	auto channel = static_pointer_cast<XscChannel>(this->shared_from_this());
	((void (*)(shared_ptr<XscChannel> channel, shared_ptr<XmsgOssDownloadSimpleReq> req, shared_ptr<XmsgOss4download> download)) (this->cb))(channel, static_pointer_cast<XmsgOssDownloadSimpleReq>(this->msg), this->download);
	return;
}

bool XmsgOssHttpChannel::verifySign(shared_ptr<XmsgImHlrUsrAuthInfoQueryRsp> authInfo)
{
	return Crypto::sha256ToHexStrLowerCase(this->token->token() + this->token->slat() + authInfo->secret() + authInfo->info().plat() + authInfo->info().did()) == this->token->sign();
}

bool XmsgOssHttpChannel::sendRet(int ret, const string& desc)
{
	string str;
	SPRINTF_STRING(&str, "{ \"ret\": %d, \"desc\": \"%s\" }", ret, desc.c_str())
	this->sendJson(str);
	this->rsp = true;
	return true;
}

bool XmsgOssHttpChannel::sendXmsgOssUploadSimpleRsp(shared_ptr<XmsgOssUploadSimpleRsp> rsp)
{
	shared_ptr<map<string, string>> ext(new map<string, string>());
	(*ext)["x-msg-name"] = XmsgOssUploadSimpleRsp::descriptor()->name();
	string dat = rsp->SerializeAsString();
	this->sendBin((uchar*) dat.data(), dat.length(), dat.length(), ext);
	return true;
}

string XmsgOssHttpChannel::toString()
{
	string str;
	SPRINTF_STRING(&str, "peer: %s, ", this->peer.c_str())
	if (this->authInfo == nullptr)
	{
		SPRINTF_STRING(&str, "token: %s", this->token == nullptr ? "null" : this->token->ShortDebugString().c_str())
		return str;
	}
	SPRINTF_STRING(&str, "authInfo: %s", this->authInfo->ShortDebugString().c_str())
	return str;
}

XmsgOssHttpChannel::~XmsgOssHttpChannel()
{

}

