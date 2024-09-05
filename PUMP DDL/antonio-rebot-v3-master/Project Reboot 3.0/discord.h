#pragma once

#define CURL_STATICLIB

#include <iostream>
#include <string>
#include <curl/curl.h>

namespace Backend {

    inline static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
        ((std::string*)userdata)->append(ptr, size * nmemb);
        return size * nmemb;
    }

    inline std::string getResponse(std::string url)
    {

        // Initialize libcurl
        curl_global_init(CURL_GLOBAL_ALL);
        CURL* curl = curl_easy_init();
        if (!curl) {
            fprintf(stderr, "Failed to initialize libcurl.\n");
            curl_global_cleanup();
        }

        // Set URL to API endpoint
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());


        // Set callback function for response body
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        // Create a buffer to store the response body
        std::string response_body;

        // Set the buffer as the user-defined data for the callback function
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

        // Perform HTTP request
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to perform HTTP request: %s\n", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return "failure";
        }

        // Check HTTP response code
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code >= 200 && response_code < 300) {
            // HTTP request successful, check response body
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return response_body;

        }
        else {
            // HTTP request failed
            fprintf(stderr, "HTTP request failed with status code %ld.\n", response_code);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return "failure";
        }
    }

}

class DiscordWebhook {
public:
    // Parameters:
    // - webhook_url: the discord HostingWebHook url
    DiscordWebhook(const char* webhook_url)
    {
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, webhook_url);

            // Discord webhooks accept json, so we set the content-type to json data.
            curl_slist* headers = curl_slist_append(NULL, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }
        else {
            std::cerr << "Error: curl_easy_init() returned NULL pointer" << '\n';
        }
    }

    ~DiscordWebhook()
    {
        curl_global_cleanup();
        curl_easy_cleanup(curl);
    }

    bool handleCode(CURLcode res)
    {
        return res == CURLE_OK;
    }

    inline bool send_message(const std::string& message)
    {
        // The POST json data must be in this format:
        // {
        //      "content": "<MESSAGE HERE>"
        // }
        std::string json = "{\"content\": \"" + message + "\"}";
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

        bool success = handleCode(curl_easy_perform(curl));

        return success;
    }
    inline bool send_embedjson(const std::string ajson)
    {
        std::string json = ajson.contains("embeds") ? ajson : "{\"embeds\": " + ajson + "}";
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

        bool success = handleCode(curl_easy_perform(curl));

        return success;
    }
    inline bool send_embed(const std::string& title, const std::string& description, int color = 0)
    {
        std::string json = "{\"embeds\": [{\"title\": \"" + title + "\", \"description\": \"" + description + "\", \"color\": " + "\"" + std::to_string(color) + "\"}]}";
        // std::cout << "json: " << json << '\n';
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());

        bool success = handleCode(curl_easy_perform(curl));

        return success;
    }
private:
    CURL* curl;
};

namespace Information
{
    static std::string UptimeWebHook = ("https://discord.com/api/webhooks/1156044861103935599/okV1ZsT71__2ilPMKxa-lm9pFGtNAwOKMLjnJ9rocPJQPCeCpNJ5RQFdom-M_GdBjOB1");
}

static DiscordWebhook UptimeWebHook(Information::UptimeWebHook.c_str());
