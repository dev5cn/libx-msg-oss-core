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

#ifndef UP_DOWN_XMSGOBJ4UPLOAD_H_
#define UP_DOWN_XMSGOBJ4UPLOAD_H_

#include <libx-msg-common-dat-struct-cpp.h>
#include <libx-msg-oss-pb.h>

class XmsgOss4upload
{
public:
	XmsgOssStorageType storageType; 
	shared_ptr<XmsgOssUploadSimpleReq> req; 
	SptrCgt cgt; 
	string oid; 
	string hashVal; 
	string storePath; 
	ullong objSize; 
	ullong storeSize; 
	shared_ptr<XmsgKv> info; 
	int fd; 
	bool finished; 
public:
	void calHash(uchar* dat, int len, bool more); 
	void evnDisc(); 
	void finish();
	string toString();
	XmsgOss4upload();
	virtual ~XmsgOss4upload();
private:
	vector<uchar> cached4hash; 
	void* sc; 
};

#endif 
