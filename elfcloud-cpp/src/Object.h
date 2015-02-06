/*** 
 * elfcloud.fi C++ Client
 * ===========================================================================
 * $Id$
 * 
 * LICENSE
 * ===========================================================================
 * Copyright 2010-2013 elfCLOUD /
 * elfcloud.fi - SCIS Secure Cloud Infrastructure Services
 * 
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *  
 *        http://www.apache.org/licenses/LICENSE-2.0
 *  
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 ***/

#ifndef ELFCLOUD_OBJECT_H_
#define ELFCLOUD_OBJECT_H_

#ifdef WINDOWS
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif

#include <memory>

namespace elfcloud {

class Object {
public:
	Object();
	virtual ~Object();
};

class Cacheable
{
protected:

public:
    Cacheable(){}
    virtual ~Cacheable(){}
    virtual void assign(std::shared_ptr<Object> pInObject)=0;
};

}

#endif /* ELFCLOUD_OBJECT_H_ */
