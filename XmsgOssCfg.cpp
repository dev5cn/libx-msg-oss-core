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

#include <libx-msg-common-dat-struct-cpp.h>
#include "XmsgOssCfg.h"

shared_ptr<XmsgOssCfg> XmsgOssCfg::cfg; 

XmsgOssCfg::XmsgOssCfg()
{

}

shared_ptr<XmsgOssCfg> XmsgOssCfg::instance()
{
	return XmsgOssCfg::cfg;
}

shared_ptr<XmsgOssCfg> XmsgOssCfg::load(const char* path)
{
	XMLDocument doc;
	if (doc.LoadFile(path) != 0)
	{
		LOG_ERROR("load config file failed, path: %s", path)
		return nullptr;
	}
	XMLElement* root = doc.RootElement();
	if (root == NULL)
	{
		LOG_ERROR("it a empty xml file? path: %s", path)
		return nullptr;
	}
	shared_ptr<XmsgOssCfg> cfg(new XmsgOssCfg());
	cfg->cfgPb.reset(new XmsgOssCfgPb());
	cfg->cgt = ChannelGlobalTitle::parse(Misc::strAtt(root, "cgt"));
	if (cfg->cgt == nullptr)
	{
		LOG_ERROR("load config file failed, cgt format error: %s", Misc::strAtt(root, "cgt").c_str())
		return nullptr;
	}
	cfg->cfgPb->set_cgt(cfg->cgt->toString());
	Misc::strAtt(root, "cfgType", cfg->cfgPb->mutable_cfgtype());
	if ("mongodb" == cfg->cfgPb->cfgtype())
	{
		XMLElement* dbUri = root->FirstChildElement("mongodb");
		auto mongodb = cfg->cfgPb->mutable_mongodb();
		Misc::strAtt(dbUri, "uri", mongodb->mutable_uri());
		XmsgOssCfg::setCfg(cfg); 
		return cfg;
	}
	if ("mysql" == cfg->cfgPb->cfgtype())
	{
		LOG_ERROR("not supported mysql.")
		return nullptr;
	}
	if ("mongodb" == Misc::strAtt(root, "db") && !cfg->loadMongodbCfg(root))
		return nullptr;
	if ("mysql" == Misc::strAtt(root, "db") && !cfg->loadMysqlCfg(root))
		return nullptr;
	if (!cfg->loadLogCfg(root))
		return nullptr;
	if (!cfg->loadXscServerCfg(root))
		return nullptr;
	if (!cfg->loadXmsgNeH2nCfg(root))
		return nullptr;
	if (!cfg->loadXmsgNeN2hCfg(root))
		return nullptr;
	if (!cfg->loadMiscCfg(root))
		return nullptr;
	LOG_INFO("load config file successful, cfg: %s", cfg->toString().c_str())
	XmsgOssCfg::setCfg(cfg);
	return cfg;
}

void XmsgOssCfg::setCfg(shared_ptr<XmsgOssCfg> cfg)
{
	XmsgOssCfg::cfg = cfg;
}

shared_ptr<XscHttpCfg> XmsgOssCfg::pubXscHttpServerCfg()
{
	if (!this->cfgPb->has_pubhttp())
		return nullptr;
	shared_ptr<XscHttpCfg> httpCfg(new XscHttpCfg());
	httpCfg->addr = this->cfgPb->pubhttp().tcp().addr();
	httpCfg->worker = this->cfgPb->pubhttp().tcp().worker();
	httpCfg->peerLimit = this->cfgPb->pubhttp().tcp().peerlimit();
	httpCfg->peerMtu = this->cfgPb->pubhttp().tcp().peermtu();
	httpCfg->peerRcvBuf = this->cfgPb->pubhttp().tcp().peerrcvbuf();
	httpCfg->peerSndBuf = this->cfgPb->pubhttp().tcp().peersndbuf();
	httpCfg->lazyClose = this->cfgPb->pubhttp().tcp().lazyclose();
	httpCfg->tracing = this->cfgPb->pubhttp().tcp().tracing();
	httpCfg->heartbeat = this->cfgPb->pubhttp().tcp().heartbeat();
	httpCfg->n2hZombie = this->cfgPb->pubhttp().tcp().n2hzombie();
	httpCfg->n2hTracing = this->cfgPb->pubhttp().tcp().n2htracing();
	httpCfg->h2nReConn = this->cfgPb->pubhttp().tcp().h2nreconn();
	httpCfg->h2nTransTimeout = this->cfgPb->pubhttp().tcp().h2ntranstimeout();
	httpCfg->headerLimit = this->cfgPb->pubhttp().headerlimit();
	httpCfg->bodyLimit = this->cfgPb->pubhttp().bodylimit();
	httpCfg->closeWait = this->cfgPb->pubhttp().closewait();
	for (auto& it : this->cfgPb->pubhttp().requiredheader())
		httpCfg->requiredHeader.insert(it);
	return httpCfg;
}

shared_ptr<XscTcpCfg> XmsgOssCfg::priXscTcpServerCfg()
{
	shared_ptr<XscTcpCfg> tcpCfg(new XscTcpCfg());
	tcpCfg->addr = this->cfgPb->pritcp().addr();
	tcpCfg->worker = this->cfgPb->pritcp().worker();
	tcpCfg->peerLimit = this->cfgPb->pritcp().peerlimit();
	tcpCfg->peerMtu = this->cfgPb->pritcp().peermtu();
	tcpCfg->peerRcvBuf = this->cfgPb->pritcp().peerrcvbuf();
	tcpCfg->peerSndBuf = this->cfgPb->pritcp().peersndbuf();
	tcpCfg->lazyClose = this->cfgPb->pritcp().lazyclose();
	tcpCfg->tracing = this->cfgPb->pritcp().tracing();
	tcpCfg->heartbeat = this->cfgPb->pritcp().heartbeat();
	tcpCfg->n2hZombie = this->cfgPb->pritcp().n2hzombie();
	tcpCfg->n2hTracing = this->cfgPb->pritcp().n2htracing();
	tcpCfg->h2nReConn = this->cfgPb->pritcp().h2nreconn();
	tcpCfg->h2nTransTimeout = this->cfgPb->pritcp().h2ntranstimeout();
	return tcpCfg;
}

bool XmsgOssCfg::loadLogCfg(XMLElement* root)
{
	auto node = root->FirstChildElement("log");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <log>");
		return false;
	}
	XmsgOssCfgLog* log = this->cfgPb->mutable_log();
	log->set_level(Misc::toUpercase(Misc::strAtt(node, "level")));
	log->set_output(Misc::toUpercase(Misc::strAtt(node, "output")));
	return true;
}

bool XmsgOssCfg::loadXscServerCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("xsc-server");
	while (node != NULL)
	{
		string name;
		Misc::strAtt(node, "name", &name);
		if ("pub-http" == name)
		{
			auto pub = this->loadXscHttpCfg(node);
			if (pub != nullptr)
				this->cfgPb->mutable_pubhttp()->CopyFrom(*pub);
			node = node->NextSiblingElement("xsc-server");
			continue;
		}
		if ("pri-tcp" == name)
		{
			auto pri = this->loadXscTcpCfg(node);
			if (pri == nullptr)
				return false;
			this->cfgPb->mutable_pritcp()->CopyFrom(*pri);
			node = node->NextSiblingElement("xsc-server");
			continue;
		}
		LOG_ERROR("unexpected xsc server name: %s, node: <xsc-server>", name.c_str())
		return false;
	}
	if (!this->cfgPb->has_pubhttp() || !this->cfgPb->has_pritcp())
	{
		LOG_ERROR("load config failed, node: <xsc-server>")
		return false;
	}
	return true;
}

shared_ptr<XmsgOssCfgXscTcpServer> XmsgOssCfg::loadXscTcpCfg(XMLElement* node)
{
	if (node == NULL)
		return nullptr;
	shared_ptr<XmsgOssCfgXscTcpServer> tcpCfg(new XmsgOssCfgXscTcpServer());
	tcpCfg->set_addr(Misc::strAtt(node, "addr"));
	tcpCfg->set_worker(Misc::hexOrInt(node, "worker"));
	tcpCfg->set_peerlimit(Misc::hexOrInt(node, "peerLimit"));
	tcpCfg->set_peermtu(Misc::hexOrInt(node, "peerMtu"));
	tcpCfg->set_peerrcvbuf(Misc::hexOrInt(node, "peerRcvBuf"));
	tcpCfg->set_peersndbuf(Misc::hexOrInt(node, "peerSndBuf"));
	tcpCfg->set_lazyclose(Misc::hexOrInt(node, "lazyClose"));
	tcpCfg->set_tracing("true" == Misc::strAtt(node, "tracing"));
	tcpCfg->set_heartbeat(Misc::hexOrInt(node, "heartbeat"));
	tcpCfg->set_n2hzombie(Misc::hexOrInt(node, "n2hZombie"));
	tcpCfg->set_n2htranstimeout(Misc::hexOrInt(node, "n2hTransTimeout"));
	tcpCfg->set_n2htracing("true" == Misc::strAtt(node, "n2hTracing"));
	tcpCfg->set_h2nreconn(Misc::hexOrInt(node, "h2nReConn"));
	tcpCfg->set_h2ntranstimeout(Misc::hexOrInt(node, "h2nTransTimeout"));
	return tcpCfg;
}

shared_ptr<XmsgOssCfgXscHttpServer> XmsgOssCfg::loadXscHttpCfg(XMLElement* node)
{
	if (node == NULL)
		return nullptr;
	shared_ptr<XmsgOssCfgXscHttpServer> httpServer(new XmsgOssCfgXscHttpServer());
	httpServer->mutable_tcp()->CopyFrom(*this->loadXscTcpCfg(node));
	httpServer->set_headerlimit(Misc::hexOrInt(node, "headerLimit"));
	httpServer->set_bodylimit(Misc::hexOrInt(node, "bodyLimit"));
	httpServer->set_closewait(Misc::hexOrInt(node, "closeWait"));
	vector<string> header;
	Misc::split(Misc::strAtt(node, "requiredHeader"), ",", header);
	for (auto& it : header)
		httpServer->add_requiredheader(it);
	return httpServer;
}

bool XmsgOssCfg::loadXmsgNeH2nCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("ne-group-h2n");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <ne-group-h2n>")
		return false;
	}
	node = node->FirstChildElement("ne");
	while (node != NULL)
	{
		auto ne = this->cfgPb->add_h2n();
		Misc::strAtt(node, "neg", ne->mutable_neg());
		Misc::strAtt(node, "addr", ne->mutable_addr());
		Misc::strAtt(node, "pwd", ne->mutable_pwd());
		Misc::strAtt(node, "alg", ne->mutable_alg());
		if (ne->neg().empty() || ne->addr().empty() || ne->alg().empty())
		{
			LOG_ERROR("load config failed, node: <ne-group-h2n><ne>, ne: %s", ne->ShortDebugString().c_str())
			return false;
		}
		node = node->NextSiblingElement("ne");
	}
	if (this->cfgPb->h2n().empty())
	{
		LOG_ERROR("load config failed, node: <ne-group-h2n><ne>")
		return false;
	}
	return true;
}

bool XmsgOssCfg::loadXmsgNeN2hCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("ne-group-n2h");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: ne-group-n2h")
		return false;
	}
	node = node->FirstChildElement("ne");
	while (node != NULL)
	{
		auto ne = this->cfgPb->add_n2h();
		Misc::strAtt(node, "neg", ne->mutable_neg());
		Misc::strAtt(node, "cgt", ne->mutable_cgt());
		Misc::strAtt(node, "pwd", ne->mutable_pwd());
		Misc::strAtt(node, "addr", ne->mutable_addr());
		SptrCgt cgt = ChannelGlobalTitle::parse(ne->cgt());
		if (ne->neg().empty() || ne->cgt().empty() || ne->pwd().empty() || cgt == nullptr)
		{
			LOG_ERROR("load config failed, node: <ne-group-n2h><ne>, ne: %s", ne->ShortDebugString().c_str())
			return false;
		}
		node = node->NextSiblingElement("ne");
	}
	if (this->cfgPb->n2h().empty())
	{
		LOG_ERROR("load config failed, node: <ne-group-n2h><ne>")
		return false;
	}
	return true;
}

bool XmsgOssCfg::loadMysqlCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("mysql");
	if (node == nullptr)
	{
		LOG_ERROR("load config failed, node: <mysql>")
		return false;
	}
	string host;
	auto mysql = this->cfgPb->mutable_mysql();
	Misc::strAtt(node, "host", &host);
	Misc::strAtt(node, "db", mysql->mutable_db());
	Misc::strAtt(node, "usr", mysql->mutable_usr());
	Misc::strAtt(node, "password", mysql->mutable_password());
	mysql->set_poolsize(Misc::hexOrInt(node, "poolSize"));
	int port;
	if (!Net::str2ipAndPort(host.c_str(), mysql->mutable_host(), &port))
	{
		LOG_ERROR("load config failed, node: <mysql>, host format error: %s", host.c_str())
		return false;
	}
	mysql->set_port(port);
	if (mysql->db().empty() || mysql->usr().empty() || mysql->password().empty())
	{
		LOG_ERROR("load config failed, node: <mysql>")
		return false;
	}
	return true;
}

bool XmsgOssCfg::loadMongodbCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("mongodb");
	if (node == nullptr)
	{
		LOG_ERROR("load config failed, node: <mongodb>")
		return false;
	}
	auto mongodb = this->cfgPb->mutable_mongodb();
	Misc::strAtt(node, "uri", mongodb->mutable_uri());
	if (mongodb->uri().empty())
	{
		LOG_ERROR("load config failed, node: <mongodb>")
		return false;
	}
	return true;
}

bool XmsgOssCfg::loadMiscCfg(XMLElement* root)
{
	XMLElement* node = root->FirstChildElement("misc");
	if (node == NULL)
	{
		LOG_ERROR("load config failed, node: <misc>")
		return false;
	}
	auto misc = this->cfgPb->mutable_misc();
	string storage = Misc::strAtt(node, "storage");
	if (storage.empty())
	{
		LOG_ERROR("load config failed, node: <misc>, missing storage")
		return false;
	}
	if ("ipfs" == storage)
	{
		misc->set_storage(XmsgOssStorageType::X_MSG_OSS_STORAGE_TYPE_IPFS);
		string ipfsNode = Misc::strAtt(node, "ipfsNode");
		if (ipfsNode.empty())
		{
			LOG_ERROR("load config failed, node: <misc>, missing ipfsNode")
			return false;
		}
		misc->set_ipfsnode(ipfsNode);
		misc->set_ipfsuploadthread(Misc::intAtt(node, "ipfsUploadThread"));
		misc->set_ipfsuploadthread(misc->ipfsuploadthread() < 1 ? 1 : misc->ipfsuploadthread());
		misc->set_ipfsdownloadthread(Misc::intAtt(node, "ipfsDownloadThread"));
		misc->set_ipfsdownloadthread(misc->ipfsdownloadthread() < 1 ? 1 : misc->ipfsdownloadthread());
	} else
		misc->set_storage(XmsgOssStorageType::X_MSG_OSS_STORAGE_TYPE_DISK);
	string objPath = Misc::strAtt(node, "objPath");
	if (objPath.empty())
	{
		LOG_ERROR("load config failed, node: <misc>, missing objPath")
		return false;
	}
	misc->set_objpath(objPath);
	misc->set_objhashsegment(Misc::hexOrInt(node, "objHashSegment"));
	misc->set_objhashsegment(misc->objhashsegment() < 1 ? 0x2000 : misc->objhashsegment());
	misc->set_objdownloadthread(Misc::hexOrInt(node, "objDownloadThread"));
	misc->set_objdownloadthread(misc->objdownloadthread() < 1 ? 1 : misc->objdownloadthread());
	misc->set_objdownloadthread(misc->objdownloadthread() > 0x40 ? 0x40 : misc->objdownloadthread());
	misc->set_objdownloadwritebufsize(Misc::hexOrInt(node, "objDownloadWriteBufSize"));
	misc->set_objdownloadwritebufsize(misc->objdownloadwritebufsize() < 0x400 ? 0x400 : misc->objdownloadwritebufsize());
	misc->set_objdownloadwritebufsize(misc->objdownloadwritebufsize() > 0x10000 ? 0x10000 : misc->objdownloadwritebufsize());
	misc->set_objinfocached(Misc::hexOrInt(node, "objInfoCached"));
	misc->set_objinfocached(misc->objinfocached() < 0 ? 0 : misc->objinfocached());
	misc->set_objinfolru(Misc::hexOrInt(node, "objInfoLru"));
	misc->set_objinfolru(misc->objinfolru() < 0 ? 0 : misc->objinfolru());
	return true;
}

string XmsgOssCfg::toString()
{
	return this->cfgPb->ShortDebugString();
}

XmsgOssCfg::~XmsgOssCfg()
{

}
