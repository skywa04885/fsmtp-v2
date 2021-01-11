#include "Classification.src.h"

namespace FSMTP::OpenAI::Classification
{
    size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string *s)
    {
        s->append(static_cast<char *>(ptr), size * nmemb);
        return size * nmemb;
    }

    std::vector<engine_t> getEngines()
    {
        Logger logger("OpenAI_GetEngines", LoggerLevel::INFO);

        // Sets the header
        std::string bearerHeader = "Authorization: Bearer ";
        bearerHeader += Global::getConfig()["openai"]["key"].asCString();

        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, bearerHeader.c_str());

        // Sends the request
        std::string response;

        CURL *curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, FSMTP_OPENAI_API_ENGINES_URL);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        logger << DEBUG << "Performing HTTP Request: '" << 
            FSMTP_OPENAI_API_ENGINES_URL << CLASSIC << ENDL;

        CURLcode res;
        if ((res = curl_easy_perform(curl)) != CURLE_OK)
        {
            logger << ERROR << "curl_easy_perform() failed: " <<
                curl_easy_strerror(res) << CLASSIC << ENDL;
        }

        // Cleans up
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        // Gets the result
        std::vector<engine_t> result = {};
        nlohmann::json parsedResponse = nlohmann::json::parse(response);
        for (auto item : parsedResponse["data"]) 
        {
            result.push_back(engine_t {
                .id = item["id"],
                .object = item["object"],
                .owner = item["owner"],
                .ready = item["ready"] 
            });
        }

        // Logs the result
        #ifdef _SMTP_DEBUG
        logger << DEBUG << "Found " << result.size() << " engines !" << 
            ENDL << CLASSIC;
        
        size_t i = 0;
        for (auto a : result) {
            logger << DEBUG << "Engine [" << i++ << "] = {" << ENDL;
            logger << "\tID: " << a.id << ENDL;
            logger << "\tObject: " << a.object << ENDL;
            logger << "\tOwner: " << a.owner << ENDL;
            logger << "\tReady: " << a.ready << ENDL;
            logger << "}" << ENDL << CLASSIC;
        }
        #endif

        return result;
    }
    
    classify_result_t classifyMessage(const std::string &email)
    {

        Logger logger("OpenAI_IsImageSafe", LoggerLevel::INFO);

        // Loads the classification file
        std::string line, contents;
        std::ifstream file("../openai_classification.txt");
        while (std::getline(file, line, '\n'))
            contents += line + '\n';
        file.close();

        contents.pop_back();

        // Creates the body
        nlohmann::json body;
        body["prompt"] = contents;
        body["max_tokens"] = 1;
        body["temperature"] = 0;
        body["n"] = 1;
        body["best_of"] = 4;
        std::string bodyString = body.dump();

        // Creates the headers
        std::string bearerHeader = "Authorization: Bearer ";
        bearerHeader += Global::getConfig()["openai"]["key"].asCString();

        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, bearerHeader.c_str());
        headers = curl_slist_append(headers, "content-type: application/json");

        // Performs the request
        std::string response;

        CURL *curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, FSMTP_OPENAI_API_CLASSIFICATION_URL);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyString.c_str());

        logger << DEBUG << "Performing HTTP Request: '" << 
            FSMTP_OPENAI_API_CLASSIFICATION_URL << CLASSIC << ENDL;

        CURLcode res;
        if ((res = curl_easy_perform(curl)) != CURLE_OK)
        {
            logger << ERROR << "curl_easy_perform() failed: " <<
                curl_easy_strerror(res) << CLASSIC << ENDL;
        }

        // Cleans up
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        // Prints response
        std::cout << nlohmann::json::parse(response).dump(1) << std::endl;

        std::string text = nlohmann::json::parse(response)["choices"][0]["text"];

        std::cout << text << std::endl;
    }
}
