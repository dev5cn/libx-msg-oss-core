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

#ifndef XMSGOSSINFOMGR_H_
#define XMSGOSSINFOMGR_H_

#include "coll/XmsgOssInfoColl.h"

class XmsgOssInfoMgr
{
public:
	void add(shared_ptr<XmsgOssInfoColl> coll); 
	shared_ptr<XmsgOssInfoColl> find(const string& oid); 
	void dida(ullong now); 
	static bool loadCb4objInfo(shared_ptr<XmsgOssInfoColl> coll); 
	static XmsgOssInfoMgr* instance();
private:
	unordered_map<string , pair<shared_ptr<XmsgOssInfoColl>, ullong >> objInfo;
	mutex lock4objInfo;
	ullong latest4cleanCache;
	ullong interval4cleanCache;
	static XmsgOssInfoMgr* inst;
	XmsgOssInfoMgr();
	virtual ~XmsgOssInfoMgr();
};

#endif 
