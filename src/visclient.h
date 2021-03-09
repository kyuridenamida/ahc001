//
// Created by kyuridenamida on 2021/03/09.
//

#ifndef AHC001_VISCLIENT_H
#define AHC001_VISCLIENT_H

#include "./picohttpclient.hpp"
void emitJson(const string &jsonString) {
    HTTPResponse response = HTTPClient::request(HTTPClient::POST, URI("http://localhost:8888/json/"), jsonString);
    if(!response.success){
        cerr << "Failed to send request" << endl;
    }
}

#endif //AHC001_VISCLIENT_H
