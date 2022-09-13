#include "CUrlRest.h"

CUrlRest::CUrlRest(string baseURL)
{
	_baseURL = baseURL;
    memset(&_data, 0, sizeof(curl_blob));
}

// Send GET <baseURL>/<username>[/arg]
// like "curl -X GET https://arg.github.com/users/microsoft/repos"
CURLcode CUrlRest::Send(string username, string arg)
{
    CURL* request = curl_easy_init();
    CURLcode response = CURLE_FAILED_INIT;

    if (request)
    {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "charset: utf-8");

        curl_easy_setopt(request, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(request, CURLOPT_USERAGENT, "ReqBin Curl Client / 1.0");
        curl_easy_setopt(request, CURLOPT_HTTPGET, 1);

        if (_data.data)
        {
            // Clear old response
            free(_data.data);
            memset(&_data, 0, sizeof(curl_blob));
        }
        curl_easy_setopt(request, CURLOPT_WRITEFUNCTION, ResponseCallback);
        curl_easy_setopt(request, CURLOPT_WRITEDATA, (void*)&_data);

        // Construct url
        string url(_baseURL + username);
        if (!arg.empty())
            url.append(arg);
        curl_easy_setopt(request, CURLOPT_URL, url.c_str());

        response = curl_easy_perform(request);

        curl_easy_cleanup(request);
    }
    return response;
}

// Writes response callback - copy data into "_data.data"
size_t CUrlRest::ResponseCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    struct curl_blob* data = (curl_blob*)userp;

    // Realoc memory block
    data->data = realloc(data->data, data->len + realsize + 1);
    if (!data->data)
    {
        return 0;
    }
    // Copy data as string
    char* str = (char*)data->data;
    memcpy(&(str[data->len]), contents, realsize);
    data->len += realsize;
    str[data->len] = 0;

    return realsize;
}

