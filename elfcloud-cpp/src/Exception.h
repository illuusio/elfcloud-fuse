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

#ifndef ELFCLOUD_EXCEPTION_H_
#define ELFCLOUD_EXCEPTION_H_

#include "Object.h"

#include <string>

namespace elfcloud {

typedef enum {
    ECSCI_EXC_UNDEFINED = 0,
    ECSCI_EXC_GENERIC_FILE_OPEN_ERROR = 3000,
    ECSCI_EXC_GENERIC_FILE_READ_ERROR,
    ECSCI_EXC_GENERIC_MEM_ALLOC_ERROR,
    ECSCI_EXC_KEYMGMT_DEFAULT_KEY_NOT_SET = 3100,
    ECSCI_EXC_KEYMGMT_KEY_NOT_FOUND,
    ECSCI_EXC_NETWORK_CONNECTION_FAILED = 3500,
    ECSCI_EXC_REQUEST_PROCESSING_FAILED = 4000,
    ECSCI_EXC_INIT_OR_PARAM_FAILURE,
    ECSCI_EXC_DUPLICATE_UNIQUE_IDENTIFIER,
    ECSCI_EXC_MEMORY_ALLOCATION_ERROR,
    ECSCI_EXC_ENCRYPTION_ERROR,
    ECSCI_EXC_CONFIG_FILE_FORMAT_ERROR,
    ECSCI_EXC_BACKEND_EXCEPTION = 9000
} ExceptionCode;

// Top level class for exception classes thrown by the elfcloud.fi Client API
class Exception: public elfcloud::Object {
private:
    ExceptionCode code;
    std::string msg;

    public:
	Exception();
    Exception(ExceptionCode pInCode, std::string pInMsg);

    std::string getMsg() {
        return msg;
    }

    ExceptionCode getCode() {
        return code;
    }

    virtual ~Exception();
};

}

#endif /* ELFCLOUD_EXCEPTION_H_ */
