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

#ifndef _LIB_DKIM_VALIDATOR_H
#define _LIB_DKIM_VALIDATOR_H

#include "../default.h"
#include "../models/Email.src.h"
#include "../general/Logger.src.h"
#include "../general/Timer.src.h"
#include "../parsers/mimev2.src.h"
#include "DKIMHeader.src.h"
#include "DKIMCanonicalization.src.h"
#include "DKIMHashes.src.h"
#include "DKIMRecord.src.h"

using namespace FSMTP::Models;

namespace FSMTP::DKIM {
  typedef enum DKIMSignatureResultType {
    DKIMSignatureValid, DKIMSignatureInvalid,
    DKIMSignatureSystemFailure, DKIMSignatureRecordNotFound
  };

  typedef struct DKIMSignatureResult {
    DKIMSignatureResultType type;
    string details;
  };

  typedef enum DKIMValidatorResultType {
    DKIMValidationPass, DKIMValidationFail,
    DKIMValidationSystemError, DKIMValidationNeutral
  };

  typedef struct DKIMValidatorResult {
    DKIMValidatorResultType type;
    string details;
  };

  class DKIMValidator {
  public:
    DKIMValidator();

    DKIMSignatureResult validateSignature(const string &signature, 
      const vector<EmailHeader> &headers, const string &body,
      const string &rawHeaders);

    DKIMValidator &validate(const string &message);

    const DKIMValidatorResult &getResult();
    string getResultString();

    ~DKIMValidator();
  private:
    DKIMValidatorResult m_Result;
    vector<DKIMSignatureResult> m_SigResults;
    Logger m_Logger;
  };
};

#endif
