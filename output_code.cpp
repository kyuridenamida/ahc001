#include <iostream>
#include <bitset>
#include <algorithm>
#include <complex>
#include <set>
#include <random>
//
// Created by kyuridenamida on 2021/03/08.
//

#ifndef AHC001_SOLUTION_H
#define AHC001_SOLUTION_H

#include <iostream>
#include <utility>
#include <vector>
#include <istream>
#include <fstream>
#include <cassert>
#include <sstream>
#include <cmath>

using namespace std;

bool overlap(int a, int b, int A, int B) {
    if (B <= a || b <= A) {
        return false;
    }
    return true;
}

int ceil(int a, int b) {
    return (a + b - 1) / b;
}

struct P {
    int x, y;

    P() { x = -1, y = -1; }

    P(int x, int y) : x(x), y(y) {}

    bool operator==(const P &rhs) const {
        return x == rhs.x &&
               y == rhs.y;
    }

    bool operator!=(const P &rhs) const {
        return !(rhs == *this);
    }

    int manhattanDist(const P &op){
        return abs(op.x - x) + abs(op.y - y);
    }
};

struct Adv {
    int id;
    P p;
    int r;
};

class Rect {
public:
    short l;
    short r;
    short d;
    short u;

    Rect() {}

    Rect(short l, short r, short d, short u) : l(l), r(r), d(d), u(u) {}

    int area() {
        return (int) (r - l) * (u - d);
    }

    static Rect onePixelRect(const P &leftDownP) {
        return Rect(leftDownP.x, leftDownP.x + 1, leftDownP.y, leftDownP.y + 1);
    }

    bool operator==(const Rect &rhs) const {
        return l == rhs.l &&
               r == rhs.r &&
               d == rhs.d &&
               u == rhs.u;
    }

    bool operator!=(const Rect &rhs) const {
        return !(rhs == *this);
    }
};

static ifstream loadFile(const string filename) {
    ifstream ifstream(filename);
    assert(ifstream.is_open());
    return ifstream;
}

struct Input {
    const int n;
    std::vector<Adv> advs;

    Input(const int n, const vector<Adv> &advs) : n(n), advs(advs) {}

public:
    static Input fromInputStream(istream &is) {
        int n;
        is >> n;
        vector<Adv> advs;
        for (int i = 0; i < n; i++) {
            int x, y, r;
            is >> x >> y >> r;
            advs.push_back({i, P(x, y), r});
        }
        assert(advs.size() == n);
        return Input(n, advs);
    }

};


struct Output {
    vector<Rect> rects;

    Output(const vector<Rect> &rects) : rects(rects) {}

    void output(ostream &os) {
        for (auto rect : rects) {
            os << rect.l << " " << rect.d << " " << rect.r << " " << rect.u << endl;
        }
    }
};


enum DIR {
    LEFT = 0,
    RIGHT = 1,
    DOWN = 2,
    UP = 3,
    LEFT_DOWN = 4,
    RIGHT_UP = 5,
    LEFT_UP = 6,
    RIGHT_DOWN = 7,
    UNKNOWN = -1

};

#endif //AHC001_SOLUTION_H

//
// Created by kyuridenamida on 2021/03/10.
//

#ifndef AHC001_TIMER_H
#define AHC001_TIMER_H

//
// Created by kyuridenamida on 2020/02/17.
//

#ifndef MARATHON_HELPERS_TIMER_HPP
#define MARATHON_HELPERS_TIMER_HPP

class timer {
public:
    virtual double time_elapsed() = 0;

    virtual bool is_TLE() = 0;

    virtual double get_time_limit() = 0;

    virtual double relative_time_elapsed() = 0;
};


class RealTimer : timer {
private:
    const static unsigned long long int cycle_per_sec = 2950000000;
    unsigned long long int begin_cycle;
    double time_limit;
    bool is_time_limit_existing;

    unsigned long long int get_cycle() {
        unsigned int low, high;
        __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
        return ((unsigned long long int) low) | ((unsigned long long int) high << 32);
    }

public:
    RealTimer(double time_limit) : time_limit(time_limit) {
        begin_cycle = get_cycle();
        is_time_limit_existing = true;
    }

    RealTimer() {
        begin_cycle = get_cycle();
        is_time_limit_existing = false;
    }

    double time_elapsed() {
        return (double) (get_cycle() - begin_cycle) / cycle_per_sec;
    }

    double get_time_limit() { return time_limit; }

    double relative_time_elapsed() { return time_elapsed() / get_time_limit(); }

    bool is_TLE() {
        assert(is_time_limit_existing);
        return time_elapsed() >= time_limit;
    }
};

#endif //MARATHON_HELPERS_TIMER_HPP


#endif //AHC001_TIMER_H

//
// Created by kyuridenamida on 2020/02/17.
//

#ifndef MARATHON_HELPERS_XORSHIFT_HPP
#define MARATHON_HELPERS_XORSHIFT_HPP

class XorShift {
public:
    XorShift() {
        initialize();
    }

    void initialize() {
        x = 123456789;
        y = 362436069;
        z = 521288629;
        w = 88675123;
    }

    // [0, ub)
    inline unsigned int next_uint32(unsigned int ub) {
        assert(ub > 0);
        return next() % ub;
    }

    // [lb, ub)
    unsigned next_uint32(unsigned int lb, unsigned int ub) {
        return lb + next_uint32(ub - lb);
    }

    // [lb, ub)
    unsigned next_int32(int lb, int ub) {
        // Need verification
        return lb + next_uint32(ub - lb);
    }

    // [0, ub)
    inline unsigned long long next_uint64(unsigned long long ub) {
        return ((1ull * next() << 32) | next()) % ub;
    }

    // [lb, ub)
    unsigned long next_uint64(unsigned long long lb, unsigned long long ub) {
        return lb + next_uint64(ub - lb);
    }

    // [0, 1.0)
    inline double next_prob() {
        return (double) next() / ((long long) 1 << 32);
    }

private:
    unsigned int x, y, z, w;

    inline unsigned int next() {
        unsigned int t = x ^x << 11;
        x = y;
        y = z;
        z = w;
        return w = w ^ w >> 19 ^ t ^ t >> 8;
    }

};

#endif //MARATHON_HELPERS_XORSHIFT_HPP
//
// Created by kyuridenamida on 2021/03/13.
//

#ifndef AHC001_VISUALIZER_H
#define AHC001_VISUALIZER_H


#include <functional>
// #include "timer.h"
//
// Created by kyuridenamida on 2021/03/09.
//

#ifndef AHC001_HTTPUTILS_H
#define AHC001_HTTPUTILS_H



/**
 * Modified by kyuridenamida to enable POST sending.
 */
/*
  picohttpclient.hpp ... generic, lightweight HTTP 1.1 client
  ... no complex features, no chunking, no ssl, no keepalive ...
  ... not very tested, use at your own risk!
  ... it PURPOSELY does not use any feature-complete libraries
      (like cURL) to stay lean and header-only.
  ... it does not use C++11 features to fit well in legacy code bases
  ... it has some suboptimal properties (like many string copy ops)
  The MIT License
  Copyright (c) 2016 Christian C. Sachs
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <sstream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// possibly add SSL?
// https://wiki.openssl.org/index.php/SSL/TLS_Client

using namespace std;

class tokenizer {
public:
    inline tokenizer(string &str) : str(str), position(0){};

    inline string next(string search, bool returnTail = false) {
        size_t hit = str.find(search, position);
        if (hit == string::npos) {
            if (returnTail) {
                return tail();
            } else {
                return "";
            }
        }

        size_t oldPosition = position;
        position = hit + search.length();

        return str.substr(oldPosition, hit - oldPosition);
    };

    inline string tail() {
        size_t oldPosition = position;
        position = str.length();
        return str.substr(oldPosition);
    };

private:
    string str;
    size_t position;
};

typedef map<string, string> stringMap;

struct URI {
    inline void parseParameters() {
        tokenizer qt(querystring);
        do {
            string key = qt.next("=");
            if (key == "")
                break;
            parameters[key] = qt.next("&", true);
        } while (true);
    }

    inline URI(string input, bool shouldParseParameters = false) {
        tokenizer t = tokenizer(input);
        protocol = t.next("://");
        string hostPortString = t.next("/");

        tokenizer hostPort(hostPortString);

        host = hostPort.next(hostPortString[0] == '[' ? "]:" : ":", true);

        if (host[0] == '[')
            host = host.substr(1, host.size() - 1);

        port = hostPort.tail();

        address = t.next("?", true);
        querystring = t.next("#", true);

        hash = t.tail();

        if (shouldParseParameters) {
            parseParameters();
        }
    };

    string protocol, host, port, address, querystring, hash;
    stringMap parameters;
};

struct HTTPResponse {
    bool success;
    string protocol;
    string response;
    string responseString;

    stringMap header;

    string body;

    inline HTTPResponse() : success(true){};
    inline static HTTPResponse fail() {
        HTTPResponse result;
        result.success = false;
        return result;
    }
};

struct HTTPClient {
    typedef enum {
        OPTIONS = 0,
        GET,
        HEAD,
        POST,
        PUT,
        DELETE,
        TRACE,
        CONNECT
    } HTTPMethod;

    inline static const char *method2string(HTTPMethod method) {
        const char *methods[] = {"OPTIONS", "GET",   "HEAD",    "POST", "PUT",
                                 "DELETE",  "TRACE", "CONNECT", NULL};
        return methods[method];
    };

    inline static int connectToURI(URI uri) {
        struct addrinfo hints, *result, *rp;

        memset(&hints, 0, sizeof(addrinfo));

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if (uri.port == "") {
            uri.port = "80";
        }

        int getaddrinfo_result =
                getaddrinfo(uri.host.c_str(), uri.port.c_str(), &hints, &result);

        if (getaddrinfo_result != 0)
            return -1;

        int fd = -1;

        for (rp = result; rp != NULL; rp = rp->ai_next) {

            fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

            if (fd == -1) {
                continue;
            }

            int connect_result = connect(fd, rp->ai_addr, rp->ai_addrlen);

            if (connect_result == -1) {
                // successfully created a socket, but connection failed. close it!
                close(fd);
                fd = -1;
                continue;
            }

            break;
        }

        freeaddrinfo(result);

        return fd;
    };

    inline static string bufferedRead(int fd) {
        size_t initial_factor = 4, buffer_increment_size = 8192, buffer_size = 0,
                bytes_read = 0;
        string buffer;

        buffer.resize(initial_factor * buffer_increment_size);

        do {
            bytes_read = read(fd, ((char *)buffer.c_str()) + buffer_size,
                              buffer.size() - buffer_size);

            buffer_size += bytes_read;

            if (bytes_read > 0 &&
                (buffer.size() - buffer_size) < buffer_increment_size) {
                buffer.resize(buffer.size() + buffer_increment_size);
            }
        } while (bytes_read > 0);

        buffer.resize(buffer_size);
        return buffer;
    };



    inline static HTTPResponse request(HTTPMethod method, URI uri, string bodyJson) {
#define HTTP_NEWLINE "\r\n"
#define HTTP_SPACE " "
#define HTTP_HEADER_SEPARATOR ": "

        int fd = connectToURI(uri);
        if (fd < 0)
            return HTTPResponse::fail();

        string request = string(method2string(method)) + string(" /") +
                         uri.address + ((uri.querystring == "") ? "" : "?") +
                         uri.querystring + " HTTP/1.1" HTTP_NEWLINE "Host: " +
                         uri.host + HTTP_NEWLINE
                                    "Accept: */*" HTTP_NEWLINE
                                    "Connection: close" HTTP_NEWLINE
                                    "Content-Length: " + itos(bodyJson.size()) + HTTP_NEWLINE
                                    "Content-Type: application/json" HTTP_NEWLINE HTTP_NEWLINE;
        request += bodyJson;
        int bytes_written = write(fd, request.c_str(), request.size());
        string buffer = bufferedRead(fd);

        close(fd);

        HTTPResponse result;

        tokenizer bt(buffer);

        result.protocol = bt.next(HTTP_SPACE);
        result.response = bt.next(HTTP_SPACE);
        result.responseString = bt.next(HTTP_NEWLINE);

        string header = bt.next(HTTP_NEWLINE HTTP_NEWLINE);

        result.body = bt.tail();

        tokenizer ht(header);

        do {
            string key = ht.next(HTTP_HEADER_SEPARATOR);
            if (key == "")
                break;
            result.header[key] = ht.next(HTTP_NEWLINE, true);
        } while (true);

        return result;
    };
private:

    static string itos(int n){
        stringstream ss;
        ss << n;
        return ss.str();
    }
};

namespace HttpUtils {
    void emitJsonToUrl(const string &url, const string &jsonString) {
        HTTPResponse response = HTTPClient::request(HTTPClient::POST, URI(url), jsonString);
        if (!response.success) {
//             cerr << "Failed to send request" << endl;
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


class Visualizer {
    RealTimer *timer;
    const string SEND_JSON_END_POINT = "http://localhost:8888/json/";
public:
    Visualizer(RealTimer *realTimer) : timer(realTimer) {
    }


    void emitJson(const string &jsonString) const {
        HttpUtils::emitJsonToUrl(SEND_JSON_END_POINT, jsonString);
    }

    void emitJsonWithTimer(std::function<string(void)> lazyJsonSupplier) {
        double now = timer->time_elapsed();
        if (now - lastCommunicationTime > 0.016) {
            // 60 rps
            emitJson(lazyJsonSupplier());
            lastCommunicationTime = now;
        }
    }

    double lastCommunicationTime = -99999;
};

template<class VisComResponse>
class VisualizerCommunicator {
    RealTimer *timer;
public:
    VisualizerCommunicator(
            string communicationFile,
            Visualizer *visualizer,
            RealTimer *timer
    ) : communicationFile(std::move(communicationFile)),
        visualizer(visualizer),
        timer(timer) {}

    static VisualizerCommunicator *start(Visualizer *visualizer, RealTimer *timer) {
        const int randomNumber = (int) (std::random_device()() % 100000);
        const string communicationFile = "/tmp/" + to_string(randomNumber) + ".com";
        auto *visualizerCommunicator =
                new VisualizerCommunicator(communicationFile, visualizer, timer);

        visualizerCommunicator->sendRegisterFileToVisualizer();

        return visualizerCommunicator;
    }

    void sendRegisterFileToVisualizer() {
        visualizer->emitJson(HttpUtils::mapToJson(
                {
                        {"type", HttpUtils::jsonValue("communication")},
                        {"file", HttpUtils::jsonValue(communicationFile)},
                }));
    }

    VisComResponse receiveResponseIfExists() {
#ifndef CLION
        return VisComResponse::empty();
#endif
        double now = timer->time_elapsed();
        if (now - lastCommunicationTime > 0.2) {
            //一応ファイルを再度Visualizerに登録する。
            sendRegisterFileToVisualizer();
            VisComResponse response = readResponseFromFileIfExists();
            lastCommunicationTime = now;
            return response;
        }
        return VisComResponse::empty();
    }

private:
    const string communicationFile;
    const Visualizer *visualizer;
    double lastCommunicationTime = -99999;

    VisComResponse readResponseFromFileIfExists() {
        ifstream ifs(communicationFile);
        if (!ifs.is_open()) {
//             cerr << "Communicating but no file found yet." << endl;
            return VisComResponse::empty();
        }
//         cerr << "Communicating and found the file!" << endl;
        auto resp = VisComResponse::readFromStream(ifs);

        // Remove the file
        remove(communicationFile.c_str());
        return resp;
    }
};

#endif //AHC001_VISUALIZER_H


using namespace std;

const double TIME_LIMIT_SECONDS = 5;

/**
 * VisualizerCommunicatorに使われます。
 */
struct AHC001VisComResponse {
    const bool received;
    const vector<int> removeIndexes;

    AHC001VisComResponse(
            const bool received,
            const vector<int> &removeIndexes) : received(received),
                                                removeIndexes(removeIndexes) {}

    static AHC001VisComResponse readFromStream(istream &is) {
        vector<int> removeIndexes;
        int idx;
        while (is >> idx) {
            removeIndexes.push_back(idx);
        }
//         cerr << "Read " << removeIndexes.size() << " delete removeIndexes" << endl;

        return AHC001VisComResponse(true, removeIndexes);
    }

    static AHC001VisComResponse empty() {
        return AHC001VisComResponse(false, {});
    }
};

using AHC001VisualizerCommunicator = VisualizerCommunicator<AHC001VisComResponse>;

struct ApplicationContext {
    RealTimer *timer;
    XorShift *rng;
    Visualizer *vis;
    AHC001VisualizerCommunicator *visCom;

    ApplicationContext(
            RealTimer *timer,
            XorShift *rng,
            Visualizer *vis,
            AHC001VisualizerCommunicator *visCom
    ) : timer(timer), rng(rng), vis(vis), visCom(visCom) {}
};

// Global declaration for handiness
ApplicationContext *ctx = nullptr;

void registerApplicationContext(ApplicationContext *applicationContext) {
    assert(ctx == nullptr);
    ctx = applicationContext;
}

string buildReportJson(vector<Rect> rects, double score, double fakeScore, Input input) {
    using namespace HttpUtils;
    auto f = [&](Rect r, int idx) {
        double h = 1 - 1. * min(r.area(), input.advs[idx].r) / max(r.area(), input.advs[idx].r);
        double subScore = 1 - h * h;
        return mapToJson(
                {
                        {"l",        jsonValue(r.l)},
                        {"r",        jsonValue(r.r)},
                        {"u",        jsonValue(r.u)},
                        {"d",        jsonValue(r.d)},
                        {"id",       jsonValue(idx)},
                        {"need",     jsonValue(input.advs[idx].r)},
                        {"px",       jsonValue(input.advs[idx].p.x)},
                        {"py",       jsonValue(input.advs[idx].p.y)},
                        {"subScore", jsonValue(subScore)}
                }
        );
    };
    stringstream rectsArray;
    rectsArray << "[";
    for (int i = 0; i < rects.size(); i++) {
        if (i != 0) {
            rectsArray << ",";
        }
        rectsArray << f(rects[i], i);
    }
    rectsArray << "]";
    return mapToJson(
            {
                    {"rects",     rectsArray.str()},
                    {"type",      jsonValue("draw")},
                    {"fakeScore", jsonValue(fakeScore)},
                    {"score",     jsonValue(score)},
            }
    );
}


struct RectSet {
private:
    double realScore;
public:
    int n;
    vector<Rect> rects;
    vector<Adv> advs;


    bool rollbackable = true;
    vector<pair<int, Rect> > prevItems;
    double prevRealScore;

    void init(vector<Rect> rects, vector<Adv> advs) {
        this->n = rects.size();
        this->rects = rects;
        this->advs = advs;
        this->realScore = realScoreFull();
        this->rollbackable = false;
    }

    inline double getRealScore() {
        return realScore;
    }

    inline double individualRealScore(int i) {
        double h = 1 - 1. * min(rects[i].area(), advs[i].r) / max(rects[i].area(), advs[i].r);
        return (1 - h * h) / n;
    }

    double realScoreFull() {
        double ans = 0;
        for (int i = 0; i < rects.size(); i++) {
            ans += individualRealScore(i);
        }
        return ans;
    }

    double score() {
        return realScore;
    }

    bool update(const int i, const Rect &geoRect_, int pushDir, int pushLength) {
        if (pushLength == 0)
            return true;
        auto geoRect = normalizedRect(geoRect_, i);
        prevItems.clear();
        prevItems.emplace_back(i, rects[i]);
        prevRealScore = realScore;
        rollbackable = true;

        realScore -= individualRealScore(i);
        rects[i] = geoRect;
        realScore += individualRealScore(i);
        bool bad = false;
        for (int j = 0; j < n; j++) {
            if (i != j) {
                auto &&op = rects[j];
                bool X = overlap(geoRect_.l, geoRect_.r, op.l, op.r);
                bool Y = overlap(geoRect_.d, geoRect_.u, op.d, op.u);
                if (X && Y) {

                    prevItems.emplace_back(j, op);
                    realScore -= individualRealScore(j);
                    if (pushDir == DIR::LEFT) {
                        op = Rect(op.l, geoRect_.l, op.d, op.u);
                    } else if (pushDir == DIR::RIGHT) {
                        op = Rect(geoRect_.r, op.r, op.d, op.u);
                    } else if (pushDir == DIR::DOWN) {
                        op = Rect(op.l, op.r, op.d, geoRect_.d);
                    } else if (pushDir == DIR::UP) {
                        op = Rect(op.l, op.r, geoRect_.u, op.u);
                    } else if (pushDir == DIR::LEFT_DOWN) {
                        op = Rect(op.l, geoRect_.l, op.d, geoRect_.d);
                    } else if (pushDir == DIR::RIGHT_UP) {
                        op = Rect(geoRect_.r, op.r, geoRect_.u, op.u);
                    } else if (pushDir == DIR::LEFT_UP) {
                        op = Rect(op.l, geoRect_.l, geoRect_.u, op.u);
                    } else if (pushDir == DIR::RIGHT_DOWN) {
                        op = Rect(geoRect_.r, op.r, op.d, geoRect_.d);
                    }
                    op = normalizedRect(op, j);

                    realScore += individualRealScore(j);

                    bool X = overlap(rects[i].l, rects[i].r, rects[j].l, rects[j].r);
                    bool Y = overlap(rects[i].d, rects[i].u, rects[j].d, rects[j].u);
                    if (X && Y) {
                        rollBack();
                        return false;
                    }
                }
            }
        }
        if (!bad) {
            if (geoRect != geoRect_) {
                for (int j = 0; j < n; j++) {
                    if (i != j) {
                        bool X = overlap(rects[i].l, rects[i].r, rects[j].l, rects[j].r);
                        bool Y = overlap(rects[i].d, rects[i].u, rects[j].d, rects[j].u);
                        if (X && Y) {
// //                            cerr << (geoRect != geoRect_) << ")" << endl;
                            rollBack();
                            return false;
                        }
                    }
                }
            }
        }
        return true;

    }

    Rect normalizedRect(Rect geoRect, int i) {
        geoRect.l = max<int>(0, geoRect.l);
        geoRect.d = max<int>(0, geoRect.d);
        geoRect.r = min<int>(10000, geoRect.r);
        geoRect.u = min<int>(10000, geoRect.u);
        auto q = advs[i].p;
        geoRect.l = min<int>(geoRect.l, q.x);
        geoRect.r = max<int>(geoRect.r, q.x + 1);
        geoRect.d = min<int>(geoRect.d, q.y);
        geoRect.u = max<int>(geoRect.u, q.y + 1);
        return geoRect;
    }

    void rollBack() {
        assert(rollbackable);
        realScore = prevRealScore;
        for (auto &&i : prevItems) {
            rects[i.first] = i.second;
        }
        rollbackable = false;
    }
};


inline Rect shake(Rect rIdx, DIR &dir_dest, int &pushLength) {
    DIR dir = static_cast<DIR>(ctx->rng->next_uint32(0, 4));
    int moveAmount = ctx->rng->next_uint32(-100, 100);
    if (dir == DIR::LEFT || dir == DIR::RIGHT) {
        if (dir == DIR::LEFT) {
            // 左伸ばす
            rIdx.l -= moveAmount;
            rIdx.r -= moveAmount;
        } else {
            rIdx.l += moveAmount;
            rIdx.r += moveAmount;
        }
    } else {
        if (dir == DIR::DOWN) {
            // した伸ばす
            rIdx.d -= moveAmount;
            rIdx.u -= moveAmount;
        } else {
            // うえ伸ばす
            rIdx.d += moveAmount;
            rIdx.u += moveAmount;
        }
    }
    pushLength = moveAmount;
    dir_dest = dir;
    return rIdx;
}

inline Rect extend(Rect newRect, DIR &dir_dest, int &diff) {
    DIR dir = static_cast<DIR>(ctx->rng->next_uint32(0, 4));
    int pushLength = ctx->rng->next_uint32(-100, 100);
    if (dir == DIR::LEFT || dir == DIR::RIGHT) {
        if (dir == DIR::LEFT) {
            // 左伸ばす
            newRect.l -= pushLength;
        } else {
            newRect.r += pushLength;
        }
    } else {
        if (dir == DIR::DOWN) {
            // した伸ばす
            newRect.d -= pushLength;
        } else {
            // うえ伸ばす
            newRect.u += pushLength;
        }
    }
    diff = pushLength;
    dir_dest = dir;
    return newRect;
}

inline Rect stickyExtend(Rect newRect, const Adv &adv, DIR &dir_dest, int &pushLength) {
    double needArea = adv.r - newRect.area();
    const int cap = 10;
    DIR dir = static_cast<DIR>(ctx->rng->next_uint32(0, 8));

    int needLength = 1e9;
    if (dir == DIR::LEFT || dir == DIR::RIGHT) {
        needLength = min(cap, ceil(needArea, (newRect.u - newRect.d)));
        if (dir == DIR::LEFT) {
            // 左伸ばす
            newRect.l -= needLength;
        } else {
            // 右伸ばす
            newRect.r += needLength;
        }
    } else if (dir == DIR::DOWN || dir == DIR::UP) {
        needLength = min(cap, ceil(needArea, (newRect.r - newRect.l)));
        if (dir == DIR::DOWN) {
            // した伸ばす
            newRect.d -= needLength;
        } else {
            // うえ伸ばす
            newRect.u += needLength;
        }
    } else if (dir == DIR::LEFT_DOWN) {
        // 左したのばす
        needLength = ctx->rng->next_uint32(1, 10);
        newRect.l -= needLength;
        newRect.d -= needLength;
    } else if (dir == DIR::RIGHT_UP) {
        // 右上のばす
        needLength = ctx->rng->next_uint32(1, 10);
        newRect.r += needLength;
        newRect.u += needLength;
    } else if (dir == DIR::LEFT_UP) {
        // 左上のばす
        needLength = ctx->rng->next_uint32(1, 10);
        newRect.l -= needLength;
        newRect.u += needLength;
    } else if (dir == DIR::RIGHT_DOWN) {
        // 右下のばす
        needLength = ctx->rng->next_uint32(1, 10);
        newRect.r += needLength;
        newRect.d -= needLength;
    }
    dir_dest = dir;
    pushLength = needLength;
    return newRect;
}

struct Args {
    static const int EXPECTED_PARAM_COUNT = 3;
    double saStartTemp = 0.0005;
    double saEndTemp = 0.000001;
    int randomSeed = 0;

    Args() {
    }

    static Args fromProgramArgs(int argc, char **argv) {
        if (argc == 1) {
            // No arg is allowed
            return Args();
        }
        // argv[0] == 'program file name'
        assert(argc == EXPECTED_PARAM_COUNT + 1);
        stringstream ssForParsing;
        for (int i = 1; i <= argc; i++) {
            ssForParsing << argv[i] << " ";
        }
        Args args;
        assert(ssForParsing >> args.saStartTemp >> args.saEndTemp >> args.randomSeed);
    }
};


Output solveBySimulatedAnnealing(Input input, const Args &args) {
    vector<Rect> rects;
    for (auto a : input.advs) {
        rects.emplace_back(a.p.x, a.p.x + 1, a.p.y, a.p.y + 1);
    }
    RectSet globalBest;
    globalBest.init(rects, input.advs);
    int iter = 0;

    vector<int> focusForcedIndexes = {};
    auto attempt = [&](RectSet &rectSet, RectSet &bestRectSet, bool emit, double t) {
        const double currentScore = rectSet.score();
        iter++;
        auto resp = ctx->visCom->receiveResponseIfExists();
        bool force = false;
        if (resp.received) {
            auto r = rectSet.rects;
            for (auto removeIndex : resp.removeIndexes) {
                r[removeIndex] = Rect::onePixelRect(input.advs[removeIndex].p);
            }
            rectSet.init(r, input.advs);
            force = true;
            focusForcedIndexes = resp.removeIndexes;
        } else {
            int targetIndex;
            if (!focusForcedIndexes.empty()) {
                targetIndex = focusForcedIndexes[ctx->rng->next_uint32(0, focusForcedIndexes.size())];
            } else {
                targetIndex = ctx->rng->next_uint32(0, input.n);
            }

            Rect newRect = rectSet.rects[targetIndex];
            DIR pushDir = DIR::UNKNOWN;
            int pushLength = -1;
            switch (ctx->rng->next_uint32(0, 3)) {
                case 0:
                    newRect = shake(newRect, pushDir, pushLength);
                    break;
                case 1:
                    newRect = extend(newRect, pushDir, pushLength);
                    break;
                case 2:
                    newRect = stickyExtend(newRect, input.advs[targetIndex], pushDir, pushLength);
                    break;
                default:
                    assert(0);
            }
            if (!rectSet.update(targetIndex, newRect, pushDir, pushLength)) {
                // Invalid, return!
                return false;
            }
        }
        double nextScore = rectSet.score();
        double p = ctx->rng->next_prob();
        bool ok = false;
        if (nextScore > 0 && (force || p < exp((nextScore - currentScore) / t))) {
            if (rectSet.getRealScore() > bestRectSet.getRealScore()) {
                ok = true;
                bestRectSet = rectSet;
            }
            if (emit) {
                auto jsonBuilder = [&]() {
                    // lazy evaluation
                    return buildReportJson(rectSet.rects, rectSet.getRealScore(), rectSet.score(), input);
                };
//                 ctx->vis->emitJsonWithTimer(jsonBuilder);
            }
        } else {
            rectSet.rollBack();
            ok = false;
        }
        return ok;
    };

    double startTemp = args.saStartTemp;
    double endTemp = args.saEndTemp;

    RectSet rectSet;
    rectSet.init(rects, input.advs);
    while (!ctx->timer->is_TLE()) {
        double temp = startTemp +
                      ctx->timer->relative_time_elapsed() * (endTemp - startTemp);
        attempt(rectSet, globalBest, true, temp);
    }
    cout << iter << endl;
    return Output(globalBest.rects);
}

void runMain(Args args, istream &is) {
    auto *timer = new RealTimer(TIME_LIMIT_SECONDS);
    auto *rng = new XorShift();
    auto *vis = new Visualizer(timer);
    AHC001VisualizerCommunicator *visCom = AHC001VisualizerCommunicator::start(vis, timer);

    registerApplicationContext(new ApplicationContext(timer, rng, vis, visCom));

    Output solution = solveBySimulatedAnnealing(Input::fromInputStream(is), args);
//    solution.output(cout);
}

int main(int argc, char *argv[]) {
    Args args = Args::fromProgramArgs(argc, argv);
#ifdef CLION
    auto inputSrc = loadFile("/home/kyuridenamida/ahc001/in/0098.txt");
    runMain(args, inputSrc);
#else
    runMain(args, cin);
#endif
}

