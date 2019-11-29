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

#include "XmsgOssInfoMgr.h"

#include "XmsgOssCfg.h"

XmsgOssInfoMgr* XmsgOssInfoMgr::inst = new XmsgOssInfoMgr();

XmsgOssInfoMgr::XmsgOssInfoMgr()
{
	this->latest4cleanCache = Xsc::clock;
	this->interval4cleanCache = 10 * DateMisc::minute;
}

XmsgOssInfoMgr* XmsgOssInfoMgr::instance()
{
	return XmsgOssInfoMgr::inst;
}

void XmsgOssInfoMgr::add(shared_ptr<XmsgOssInfoColl> coll)
{
	unique_lock<mutex> lock(this->lock4objInfo);
	this->objInfo[coll->oid] = make_pair<>(coll, DateMisc::gotoGmt0(Xsc::clock));
}

shared_ptr<XmsgOssInfoColl> XmsgOssInfoMgr::find(const string& oid)
{
	unique_lock<mutex> lock(this->lock4objInfo);
	auto it = this->objInfo.find(oid);
	if (it == this->objInfo.end())
		return nullptr;
	it->second.second = DateMisc::gotoGmt0(Xsc::clock); 
	return it->second.first;
}

bool XmsgOssInfoMgr::loadCb4objInfo(shared_ptr<XmsgOssInfoColl> coll)
{
	XmsgOssInfoMgr::instance()->objInfo[coll->oid] == make_pair<>(coll, DateMisc::gotoGmt0(Xsc::clock) );
	return true;
}

void XmsgOssInfoMgr::dida(ullong now)
{
	if (this->latest4cleanCache + this->interval4cleanCache < now) 
		return;
	ullong ts = now - XmsgOssCfg::instance()->cfgPb->misc().objinfolru() * DateMisc::hour;
	int count = 0;
	unique_lock<mutex> lock(this->lock4objInfo);
	for (auto it = this->objInfo.begin(); it != this->objInfo.end();)
	{
		if (it->second.second + ts > now) 
		{
			++it;
			continue;
		}
		++count;
		this->objInfo.erase(it++);
	}
	if (count < 1)
		return;
	lock.unlock();
	LOG_DEBUG("have some object-info over the cached time, we will remove them, count: %d", count)
}

XmsgOssInfoMgr::~XmsgOssInfoMgr()
{

}

