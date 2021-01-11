#ifndef _LIB_OPENAI_CLASSIFICATION_H
#define _LIB_OPENAI_CLASSIFICATION_H

#include "../default.h"
#include "../general/Logger.src.h"
#include "../general/Global.src.h"
#include <curl/curl.h>

#define FSMTP_OPENAI_API_CLASSIFICATION_URL "https://api.openai.com/v1/engines/davinci/completions"
#define FSMTP_OPENAI_API_ENGINES_URL "https://api.openai.com/v1/engines"

namespace FSMTP::OpenAI::Classification
{
    typedef struct {
        std::string id, object, owner;
        bool ready;
    } engine_t;

    typedef enum {
        CLASSIFY_RESULT_SPAM,
        CLASSIFY_RESULT_PERSONAL,
        CLASSIFY_RESULT_SHOPPING
    } classify_result_t;

    std::vector<engine_t> getEngines();

    classify_result_t classifyMessage(const std::string &email);
}

#endif
