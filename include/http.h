// MinIO C++ Library for Amazon S3 Compatible Cloud Storage
// Copyright 2022 MinIO, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _MINIO_HTTP_H
#define _MINIO_HTTP_H

#include <arpa/inet.h>

#include <curlpp/Easy.hpp>
#include <curlpp/Multi.hpp>
#include <curlpp/Options.hpp>

#include "utils.h"

namespace minio {
namespace http {
enum class Method { kGet, kHead, kPost, kPut, kDelete };

// MethodToString converts http Method enum to string.
/*constexpr*/ const char* MethodToString( Method& method ) throw();

/**
 * Url represents HTTP URL and it's components.
 */
struct Url
{
    Url() = default;
    Url( bool _https, std::string _host, unsigned int _port, std::string _path, std::string _query_string )
        : https( _https), host(std::move(_host)), port(_port), path(std::move(_path)), query_string(std::move(_query_string))
    {}
    ~Url() = default;

  bool https;
  std::string host;
  unsigned int port = 0;
  std::string path;
  std::string query_string;

  operator bool() const { return !host.empty(); }

  std::string String() {
    if (host.empty()) return "";

    std::string url = (https ? "https://" : "http://") + host;
    if (port) url += ":" + std::to_string(port);
    if (!path.empty()) {
      if (path.front() != '/') url += '/';
      url += path;
    }
    if (!query_string.empty()) url += "?" + query_string;

    return url;
  }

  std::string HostHeaderValue() {
    if (!port) return host;
    return host + ":" + std::to_string(port);
  }

  static Url Parse(std::string value) {
    std::string scheme;
    size_t pos = value.find("://");
    if (pos != std::string::npos) {
      scheme = value.substr(0, pos);
      value.erase(0, pos + 3);
    }
    scheme = utils::ToLower(scheme);

    if (!scheme.empty() && scheme != "http" && scheme != "https") return Url{};

    bool https = (scheme.empty() || scheme == "https");

    std::string host;
    std::string path;
    std::string query_string;
    pos = value.find("/");
    if (pos != std::string::npos) {
      host = value.substr(0, pos);
      value.erase(0, pos + 1);

      pos = value.find("?");
      if (pos != std::string::npos) {
        path = value.substr(0, pos);
        value.erase(0, pos + 1);
        query_string = value;
      } else {
        path = value;
      }
    } else {
      pos = value.find("?");
      if (pos != std::string::npos) {
        host = value.substr(0, pos);
        value.erase(0, pos + 1);
        query_string = value;
      } else {
        host = value;
      }
    }

    if (host.empty()) return Url{};

    unsigned int port = 0;
    struct sockaddr_in6 dst;
    if (inet_pton(AF_INET6, host.c_str(), &(dst.sin6_addr)) <= 0) {
      if (host.front() != '[' || host.back() != ']') {
        std::stringstream ss(host);
        std::string portstr;
        while (std::getline(ss, portstr, ':')) {
        }

        if (!portstr.empty()) {
          try {
            port = std::stoi(portstr);
            host = host.substr(0, host.rfind(":" + portstr));
          } catch (std::invalid_argument) {
            port = 0;
          }
        }
      }
    } else {
      host = "[" + host + "]";
    }

    if (!https && port == 80) port = 0;
    if (https && port == 443) port = 0;

    return Url{https, host, port, path, query_string};
  }
};  // struct Url

struct DataFunctionArgs;

using DataFunction = std::function<bool(DataFunctionArgs)>;

struct ProgressFunctionArgs;

using ProgressFunction = std::function<void(ProgressFunctionArgs)>;

struct Response;

struct DataFunctionArgs {
    DataFunctionArgs() = default;
    DataFunctionArgs( curlpp::Easy* handle_, Response* response_, std::string datachunk_, void* userdata_ )
        : handle( handle_ ), response( response_ ), datachunk( datachunk_ ), userdata( userdata_ )
    {}
        
  curlpp::Easy* handle = NULL;
  Response* response = NULL;
  std::string datachunk;
  void* userdata = NULL;
};  // struct DataFunctionArgs

struct ProgressFunctionArgs {
  double download_total_bytes = 0;
  double downloaded_bytes = 0;
  double upload_total_bytes = 0;
  double uploaded_bytes = 0;
  double download_speed = 0;
  double upload_speed = 0;
  void* userdata = NULL;
};  // struct ProgressFunctionArgs

struct Request {
  Method method;
  http::Url url;
  utils::Multimap headers;
  std::string body;
  DataFunction datafunc = NULL;
  void* userdata = NULL;
  ProgressFunction progressfunc = NULL;
  void* progress_userdata = NULL;
  bool debug = false;
  bool ignore_cert_check = false;
  std::string ssl_cert_file;
  std::string key_file;
  std::string cert_file;

  Request(Method method, Url url);
  Response Execute();
  operator bool() const {
    if (method < Method::kGet || method > Method::kDelete) return false;
    return url;
  }

 private:
  Response execute();
};  // struct Request

struct Response {
  std::string error;
  DataFunction datafunc = NULL;
  void* userdata = NULL;
  int status_code = 0;
  utils::Multimap headers;
  std::string body;

  size_t ResponseCallback(curlpp::Multi* requests, curlpp::Easy* request,
                          char* buffer, size_t size, size_t length);
  operator bool() const {
    return error.empty() && status_code >= 200 && status_code <= 299;
  }
  error::Error Error() {
    if (!error.empty()) return error::Error(error);
    if (status_code && (status_code < 200 || status_code > 299)) {
      return error::Error("failed with HTTP status code " +
                          std::to_string(status_code));
    }
    return error::SUCCESS;
  }

 private:
  std::string response_;
  bool continue100_ = false;
  bool status_code_read_ = false;
  bool headers_read_ = false;

  error::Error ReadStatusCode();
  error::Error ReadHeaders();
};  // struct Response
}  // namespace http
}  // namespace minio
#endif  // #ifndef _MINIO_HTTP_H
