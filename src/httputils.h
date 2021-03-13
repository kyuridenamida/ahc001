//
// Created by kyuridenamida on 2021/03/09.
//

#ifndef AHC001_HTTPUTILS_H
#define AHC001_HTTPUTILS_H

#include "./picohttpclient.hpp"

namespace HttpUtils {
    void emitJsonToUrl(const string &url, const string &jsonString) {
        HTTPResponse response = HTTPClient::request(HTTPClient::POST, URI(url), jsonString);
        if (!response.success) {
            cerr << "Failed to send request" << endl;
        }
    }

    string quoted(string key) {
        return "\"" + key + "\"";
    };

    typedef string JsonValue;

    JsonValue jsonValue(string x) {
        return quoted(x);
    }

    JsonValue jsonValue(int x) {
        return to_string(x);
    }

    JsonValue jsonValue(double x) {
        return to_string(x);
    }

    string mapToJson(const map<string, JsonValue> &jsonMap) {
        stringstream ss;
        for (const auto &kv : jsonMap) {
            ss << quoted(kv.first) << ":" << kv.second << ",";
        }

        string content = ss.str();
        content.pop_back(); // 末尾カンマ消す
        return "{" + content + "}";
    }

}


#endif //AHC001_HTTPUTILS_H
