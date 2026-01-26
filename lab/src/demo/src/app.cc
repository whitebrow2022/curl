#include <curl/curl.h>
#include <iostream>
#include <string>

size_t header_callback(char* buffer, size_t size, size_t nitems, void* userdata)
{
    size_t total = size * nitems;
    std::string* headers = static_cast<std::string*>(userdata);
    headers->append(buffer, total);
    return total;
}

int main()
{
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "curl init failed\n";
        return 1;
    }

    std::string headers;

    curl_easy_setopt(curl, CURLOPT_URL, "https://www.baidu.com");
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);          // 只请求头
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headers);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // 处理 301/302
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "curl error: " << curl_easy_strerror(res) << "\n";
    } else {
        std::cout << "=== HTTP Headers ===\n";
        std::cout << headers;
    }

    curl_easy_cleanup(curl);
    return 0;
}
