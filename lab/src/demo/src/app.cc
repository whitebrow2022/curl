#if WIN32
#include <windows.h>
#include <winhttp.h>
#endif

//
#include <curl/curl.h>

//
#include <iostream>
#include <string>

#if defined(USE_BORINGSSL)
#include "cacert.h"
#endif

#if WIN32
std::wstring Utf8ToUtf16(const std::string& str) {
  if (str.empty()) {
    return L"";
  }

  int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);

  std::wstring result(size_needed, 0);

  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &result[0], size_needed);

  return result;
}
#endif

std::string GetProxyForUrl(const std::string& url_str) {
#if WIN32
  std::wstring url = Utf8ToUtf16(url_str);
  WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ie_config{};
  WinHttpGetIEProxyConfigForCurrentUser(&ie_config);

  HINTERNET hsession =
      WinHttpOpen(L"curl_ua", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (!hsession) {
    return "";
  };

  WINHTTP_AUTOPROXY_OPTIONS auto_proxy{};
  auto_proxy.fAutoLogonIfChallenged = TRUE;

  if (ie_config.lpszAutoConfigUrl && *ie_config.lpszAutoConfigUrl) {
    auto_proxy.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
    auto_proxy.lpszAutoConfigUrl = ie_config.lpszAutoConfigUrl;
  } else {
    auto_proxy.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
    auto_proxy.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
  }

  WINHTTP_PROXY_INFO proxy_info{};
  BOOL ok = WinHttpGetProxyForUrl(hsession, url.c_str(), &auto_proxy, &proxy_info);

  std::string proxy_str;

  if (ok && proxy_info.lpszProxy) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, proxy_info.lpszProxy, -1, nullptr, 0, nullptr, nullptr);
    char* buffer = new char[size_needed];
    WideCharToMultiByte(CP_UTF8, 0, proxy_info.lpszProxy, -1, buffer, size_needed, nullptr, nullptr);
    proxy_str = buffer;
    delete[] buffer;
  } else if (ie_config.lpszProxy && *ie_config.lpszProxy) {
    // fallback：使用手动代理设置
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, ie_config.lpszProxy, -1, nullptr, 0, nullptr, nullptr);
    char* buffer = new char[size_needed];
    WideCharToMultiByte(CP_UTF8, 0, ie_config.lpszProxy, -1, buffer, size_needed, nullptr, nullptr);
    proxy_str = buffer;
    delete[] buffer;
  }

  if (proxy_info.lpszProxy) {
    GlobalFree(proxy_info.lpszProxy);
  }
  if (proxy_info.lpszProxyBypass) {
    GlobalFree(proxy_info.lpszProxyBypass);
  }
  WinHttpCloseHandle(hsession);
  if (ie_config.lpszProxy) {
    GlobalFree(ie_config.lpszProxy);
  }
  if (ie_config.lpszProxyBypass) {
    GlobalFree(ie_config.lpszProxyBypass);
  }
  if (ie_config.lpszAutoConfigUrl) {
    GlobalFree(ie_config.lpszAutoConfigUrl);
  }

  return proxy_str;
#else
  std::cout << "not implemented: " << url_str;
  return "";
#endif
}

size_t header_callback(char* buffer, size_t size, size_t nitems, void* userdata) {
  size_t total = size * nitems;
  std::string* headers = static_cast<std::string*>(userdata);
  headers->append(buffer, total);
  return total;
}

int main() {
  std::cout << "curl version: \n\t" << curl_version() << std::endl << std::endl;

  CURL* curl = curl_easy_init();
  if (!curl) {
    std::cerr << "curl init failed\n";
    return 1;
  }

  std::string headers;

  // 1️ get proxy
  std::string url_str = R"(https://www.google.com/)";
  std::string proxy = GetProxyForUrl(url_str);
  if (!proxy.empty()) {
    std::cout << "Using proxy: " << proxy << std::endl;
    curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());

#if defined(USE_SCHANNEL)
    // fix: * schannel: failed to receive handshake, SSL/TLS connection failed
    //
    {
      curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
      curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, 1L);

      curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NO_REVOKE);
      curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    }
#endif
  } else {
    std::cout << "No proxy detected, connecting directly" << std::endl;
  }

  // 2️ set libcurl
  curl_easy_setopt(curl, CURLOPT_URL, url_str.c_str());
  curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headers);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
  curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

  // https verify
#if defined(USE_BORINGSSL)
  struct curl_blob blob;
  blob.data = cacert_pem;
  blob.len = cacert_pem_len;
  blob.flags = CURL_BLOB_COPY;

  curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, &blob);
#endif
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

#if WIN32
#if defined(USE_SCHANNEL)
  // fix: schannel: next InitializeSecurityContext failed:
  //      CRYPT_E_REVOCATION_OFFLINE(0x80092013)
  curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NO_REVOKE);
#endif
#endif

  // debug info
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

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
