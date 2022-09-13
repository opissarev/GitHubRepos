#include <string>
#include <curl/curl.h>
using namespace std;

// Sends rest GET via "curl library" and return response on "GetData"
class CUrlRest
{
private:
	string _baseURL;
	curl_blob _data;

public:
	CUrlRest(string baseURL);
	virtual ~CUrlRest() { free(_data.data); }
	char* GetData() { return (char*)_data.data; }
	CURLcode Send(string username, string arg);
private:
	static size_t ResponseCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

