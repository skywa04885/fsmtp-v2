/*
	Copyright [2020] [Luke A.C.A. Rieff]

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

#ifndef _LIB_SPF_VALIDATOR_H
#define _LIB_SPF_VALIDATOR_H

#include "../default.h"
#include "SPFRecord.src.h"
#include "../networking/Address.src.h"
#include "../general/Logger.src.h"

namespace FSMTP::SPF {
  typedef enum SPFValidatorResultType {
    ResultTypeDenied, ResultTypeAllowed,
    ResultTypeSystemFailure
  };

  typedef struct SPFValidatorResult {
    SPFValidatorResultType type;
    string details;
  };

  class SPFValidator {
  public:
    SPFValidator();

    bool validate(const string &query, const string &cmp);
    const SPFValidatorResult &getResult();
    string getResultString();

    ~SPFValidator();
  private:
    SPFValidatorResult m_Result;
    Logger m_Logger;
  };
}

#endif
