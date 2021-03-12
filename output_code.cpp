#include <iostream>
#include <bitset>
#include <algorithm>
#include <complex>
//
// Created by kyuridenamida on 2021/03/08.
//

#ifndef AHC001_SOLUTION_H
#define AHC001_SOLUTION_H

#include <iostream>
#include <vector>
#include <istream>
#include <fstream>
#include <cassert>
#include <sstream>
#include <cmath>

using namespace std;

struct P {
    int x, y;

    P() { x = -1, y = -1; }

    P(int x, int y) : x(x), y(y) {}

    bool invalid() {
        return x == -1;
    }

    bool operator==(const P &rhs) const {
        return x == rhs.x &&
               y == rhs.y;
    }

    bool operator!=(const P &rhs) const {
        return !(rhs == *this);
    }
};

struct Size {
    int x, y;

    Size() {}

    Size(int x, int y) : x(x), y(y) {}


    int area() {
        return x * y;
    }

    static Size bestWithX(int x, int r) {
        assert(x > 0);
        return {x, (r + x - 1) / x};
    }

    static Size bestWithY(int y, int r) {
        assert(y > 0);
        return {(r + y - 1) / y, y};
    }

    static Size almostSquare(int r) {
        int S = sqrt(r) + 0.5;
        if (S * S >= r) {
            return {S, S};
        }
        assert(S * (S + 1) >= r);
        return {S, S + 1};
    }

    bool operator==(const Size &rhs) const {
        return x == rhs.x &&
               y == rhs.y;
    }

    bool operator!=(const Size &rhs) const {
        return !(rhs == *this);
    }
};

struct Adv {
    int id;
    P p;
    int r;
};

struct Rect {
    P p; // 左上の点
    Size size;

    Rect() {}

    Rect(const P &p, const Size &size) : p(p), size(size) {}

    bool operator==(const Rect &rhs) const {
        return p == rhs.p &&
               size == rhs.size;
    }

    bool operator!=(const Rect &rhs) const {
        return !(rhs == *this);
    }

    bool invalid() {
        return p.invalid();
    }
};

struct OutputItem {
    Adv adv;
    Rect r;

    OutputItem(const Adv &adv, const Rect &r) : adv(adv), r(r) {}
};

static ifstream loadFile(const string filename) {
    ifstream ifstream(filename);
    assert(ifstream.is_open());
    return ifstream;
}


// 普通に200コと当たり判定したほうがはやい、説!
// 必ず階段になるから凸型に削るがいい、説!
const int MAX_SIZE = 10005;
namespace operations {
    vector<Rect> currentRects;

    short seg00[MAX_SIZE][MAX_SIZE];
    short segXY[MAX_SIZE][MAX_SIZE];
    short seg0Y[MAX_SIZE][MAX_SIZE];
    short segX0[MAX_SIZE][MAX_SIZE];


    void add(short seg[MAX_SIZE][MAX_SIZE], const P &p, int c, int v) {
        v *= c;
        for (int i = p.y + 1; i < MAX_SIZE; i += i & -i) {
            for (int j = p.x + 1; j < MAX_SIZE; j += j & -j) {
                seg[i][j] += v;
            }
        }
    }

    int get(short seg[MAX_SIZE][MAX_SIZE], const P &p) {
        int ans = 0;
        for (int i = p.y; i > 0; i -= i & -i) {
            for (int j = p.x; j > 0; j -= j & -j) {
                ans += seg[i][j];
            }
        }
        return ans;
    }

    inline int cellCountSub(const P &p) {
        const int constSum = get(seg00, P(p.x, p.y));
        const int xCoef = get(segX0, P(p.x, p.y));
        const int yCoef = get(seg0Y, P(p.x, p.y));
        const int xy = get(segXY, P(p.x, p.y));
        return constSum + xCoef * (p.x) + yCoef * (p.y) + (p.x) * (p.y) * xy;
    }


    int cellCount(const Rect &r) {
        const int x1 = r.p.x;
        const int y1 = r.p.y;
        const int x2 = r.p.x + r.size.x;
        const int y2 = r.p.y + r.size.y;

        int sum = 0;
        sum += cellCountSub(P(x1, y1));
        sum -= cellCountSub(P(x1, y2));
        sum -= cellCountSub(P(x2, y1));
        sum += cellCountSub(P(x2, y2));
        return sum;
    }

    void change(const Rect &r, int v) {
        const int x1 = r.p.x;
        const int y1 = r.p.y;
        const int x2 = r.p.x + r.size.x;
        const int y2 = r.p.y + r.size.y;

        add(seg00, P(x1, y1), v, x1 * y1);
        add(seg00, P(x2, y1), v, -y1 * x2);
        add(seg00, P(x1, y2), v, -x1 * y2);
        add(seg00, P(x2, y2), v, x2 * y2);

        add(segXY, P(x1, y1), v, +1);
        add(segXY, P(x2, y1), v, -1);
        add(segXY, P(x1, y2), v, -1);
        add(segXY, P(x2, y2), v, +1);

        add(seg0Y, P(x1, y1), v, -x1);
        add(seg0Y, P(x2, y1), v, x2);
        add(seg0Y, P(x1, y2), v, +x1);
        add(seg0Y, P(x2, y2), v, -x2);

        add(segX0, P(x1, y1), v, -y1);
        add(segX0, P(x2, y1), v, +y1);
        add(segX0, P(x1, y2), v, +y2);
        add(segX0, P(x2, y2), v, -y2);
    }


    void fill(const Rect &r) {
        currentRects.push_back(r);
        change(r, 1);
    }

    void undo(const Rect &r) {
        change(r, -1);
    }

    void resetAll() {
        for (auto r : currentRects) {
            undo(r);
        }
        currentRects.clear();
    }
}

struct Input {
    const int n;
    std::vector<Adv> advs;
    const int W = 10000;
    const int H = 10000;

    Input(const int n, const vector<Adv> &advs) : n(n), advs(advs) {}

public:
    void outputToStream(ostream &ofs) const {
        ofs << "n = " << n << endl;
        auto fill = [](int a, int n) {
            int at = a;
            stringstream ss;
            if (a == 0) {
                ss << string(n - 1, ' ');
            } else {
                while (a > 0) {
                    n--;
                    a /= 10;
                }
                ss << string(max(0, n), ' ');
            }
            ss << at;
            return ss.str();
        };
        for (auto i : advs) {
            ofs << "id=" << fill(i.id, 3) << " (" << fill(i.p.x, 4) << "," << fill(i.p.y, 4) << ")" << " r="
                << fill(i.r, 7) << endl;
        }
    }

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
    Input input;
    std::vector<OutputItem> items;

    Output(const Input &input, const vector<OutputItem> &items) : input(input), items(items) {}


    double relativeScore() {
        return score() / input.n;
    }

    double score() {
        double p = 0;
        for (auto i : items) {
            p += 1. * min(i.adv.r, i.r.size.area()) / max(i.adv.r, i.r.size.area());
        }
        return p;
    }

    void output(ostream &os) {
        // TODO: 座標のinclusive / exclusive
        auto cpy = items;
        vector<Rect> answer(input.n);


        for (auto item : items) {
            answer[item.adv.id] = item.r;
        }

        vector<Rect> rs;

        auto getSomeEmpty = [&]() {
            for (int i = 0; i < input.H; i++) {
                for (int j = 0; j < input.W; j++) {
                    auto rect = Rect(P(j, i), Size(1, 1));
                    if (operations::cellCount(rect) == 0) {
                        return rect;
                    }
                }
            }
            assert("Game over" != "");
        };
        for (auto r : answer) {
            if (r.invalid()) {
                auto alt = getSomeEmpty();
                operations::fill(alt);
                rs.push_back(alt);
                r = alt;
            }
            os << r.p.x << " " << r.p.y << " " << r.p.x + r.size.x << " "
               << r.p.y + r.size.y << endl;
        }
        for (auto r : rs) {
            operations::undo(r);
        }
    }

    void outputDetails(ostream &os) {
        for (auto item : items) {
            os << "(" << item.r.p.x << "," << item.r.p.y << ")" << " size=(" << item.r.size.x << ","
               << item.r.size.y << ")" << " area=" << item.r.size.area() << endl;
        }
    }
};


class Solver {
public:
    virtual Output solve(Input request) = 0;
};

#endif //AHC001_SOLUTION_H

//
// Created by kyuridenamida on 2021/03/09.
//

#ifndef AHC001_VISCLIENT_H
#define AHC001_VISCLIENT_H



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
void emitJson(const string &jsonString) {
    HTTPResponse response = HTTPClient::request(HTTPClient::POST, URI("http://localhost:8888/json/"), jsonString);
    if(!response.success){
        cerr << "Failed to send request" << endl;
    }
}

#endif //AHC001_VISCLIENT_H

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

class Timer {
public:
    virtual double time_elapsed() = 0;

    virtual bool is_TLE() = 0;

    virtual double get_time_limit() = 0;

    virtual double relative_time_elapsed() = 0;
};


class RealTimer : Timer {
private:
    const static unsigned long long int cycle_per_sec = 2800000000;
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


using namespace std;

RealTimer timer(5.0);

typedef complex<double> GeoPoint;
typedef GeoPoint V;

int random(int a, int b) {
    return rand() % (b - a + 1) + a;
}

bool overlap(int a, int b, int A, int B) {
    if (B <= a || b <= A) {
        return false;
    }
    return true;
}

int overlapLength(int a, int b, int A, int B) {
    return max(min(b, B) - max(a, A), 0);
}

inline bool eq(int a, int b) {
    return a == b;
}

class GeoRect {

public:
    int l;
    int r;
    int d;
    int u;

    GeoRect() { l = r = d = u - 1; }

    GeoRect(int l, int r, int d, int u) : l(l), r(r), d(d), u(u) {}

    int area() {
        return (r - l) * (u - d);
    }

    bool operator==(const GeoRect &rhs) const {
        return eq(l, rhs.l) &&
               eq(r, rhs.r) &&
               eq(d, rhs.d) &&
               eq(u, rhs.u);
    }

    bool operator!=(const GeoRect &rhs) const {
        return !(rhs == *this);
    }

    Rect toRect() {
        return Rect(P((int) l, (int) d), Size((int) (r - l), (int) (u - d)));
    }
};


string globalCommunicationFile;

void registerCommunicationFile(const string communicationFile) {
    auto quoted = [&](string key) {
        return "\"" + key + "\"";
    };
    stringstream ss;
    ss << "{"
       << quoted("type") << ":" << quoted("communication") << ","
       << quoted("file") << ":" << quoted(communicationFile)
       << "}";
    emitJson(ss.str());
    globalCommunicationFile = communicationFile;
}

vector<int> removeIndexes() {
    registerCommunicationFile(globalCommunicationFile);
    cerr << "Communicating...";
    ifstream ifs(globalCommunicationFile);
    if (!ifs.is_open()) {
        cerr << "no file found yet." << endl;
        return {};
    };
    vector<int> indexes;
    int idx;
    while (ifs >> idx) {
        indexes.push_back(idx);
    }
    ifs.close();
    cerr << " and you have " << indexes.size() << " delete indexes" << endl;

    ofstream ofs(globalCommunicationFile);
    ofs.close();

    return indexes;
}

double _lstsub2 = -1;

vector<int> removeIndexesWithTimer() {
#ifndef CLION
    return {};
#endif
    double now = timer.time_elapsed();
    if (now - _lstsub2 > 0.2) {
        auto array = removeIndexes();
        _lstsub2 = now;
        return array;
    }
    return {};
}

string createJson(vector<GeoRect> rects, double score, Input input) {
    auto quoted = [&](string key) {
        return "\"" + key + "\"";
    };

    auto f = [&](GeoRect r, int idx) {
        stringstream ss;
        double h = 1 - 1. * min(r.area(), input.advs[idx].r) / max(r.area(), input.advs[idx].r);
        double subScore = 1 - h * h;

        ss << "{"
           << quoted("l") << ":" << r.l << ","
           << quoted("r") << ":" << r.r << ","
           << quoted("u") << ":" << r.u << ","
           << quoted("d") << ":" << r.d << ","
           << quoted("id") << ":" << idx << ","
           << quoted("need") << ":" << input.advs[idx].r << ","
           << quoted("px") << ":" << input.advs[idx].p.x << ","
           << quoted("py") << ":" << input.advs[idx].p.y << ","

           << quoted("subScore") << ":" << subScore
           << "}";
        return ss.str();
    };
    stringstream ss;
    ss << "{" << quoted("rects") << ": [";
    for (int i = 0; i < rects.size(); i++) {
        if (i != 0) {
            ss << ",\n";
        }
        ss << f(rects[i], i);
    }
    ss << "],\n";
    ss << quoted("type") << ":" << quoted("draw") << ",\n";
    ss << quoted("score") << ": " << score << "\n";
    ss << "}";
    return ss.str();
}

Output createOutput(vector<GeoRect> rects, Input input) {
    vector<OutputItem> res;
    for (auto i : input.advs) {
        res.emplace_back(i, rects[i.id].toRect());
    }
    return Output(input, res);
}

int ceil(int a, int b) {
    return (a + b - 1) / b;
}

double _lstsub = -1;

void emitJsonWithTimer(vector<GeoRect> rects, double s, Input input) {
    double now = timer.time_elapsed();
    if (now - _lstsub > 0.1) {
        emitJson(createJson(rects, s, input));
        _lstsub = now;
    }
}

inline GeoRect shrink(GeoRect rIdx, const Adv &adv) {
    auto q = adv.p;
    rIdx.l = q.x;
    rIdx.r = q.x + 1;
    rIdx.d = q.y;
    rIdx.u = q.y + 1;
    return rIdx;
}


struct RectSet {
    int n;
    vector<GeoRect> rects;
    vector<Adv> advs;
    double realScore;

    bool rollbackable = true;
    vector<pair<int, GeoRect> > prevItems;
    double prevRealScore;

    void init(vector<GeoRect> rects, vector<Adv> advs) {
        this->n = rects.size();
        this->rects = rects;
        this->advs = advs;
        this->realScore = realScoreFull();
        this->rollbackable = false;
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

    double strictScore() {
        return realScore;
    }


    void update(int i, const GeoRect &geoRect_, int dir) {
        auto geoRect = normalizedRect(geoRect_, i);
        prevItems.clear();
        prevItems.emplace_back(i, rects[i]);
        prevRealScore = realScore;
        rollbackable = true;

        realScore -= individualRealScore(i);
        rects[i] = geoRect;
        realScore += individualRealScore(i);

        for (int j = 0; j < rects.size(); j++) {
            if (i != j) {
                bool X = overlap(geoRect_.l, geoRect_.r, rects[j].l, rects[j].r);
                bool Y = overlap(geoRect_.d, geoRect_.u, rects[j].d, rects[j].u);
                if (X && Y) {
                    prevItems.emplace_back(j, rects[j]);
                    realScore -= individualRealScore(j);
                    if (dir == 0) {
                        rects[j].r = geoRect_.l;
                    } else if (dir == 1) {
                        rects[j].l = geoRect_.r;
                    } else if (dir == 2) {
                        rects[j].u = geoRect_.d;
                    } else if (dir == 3) {
                        rects[j].d = geoRect_.u;
                    }
                    rects[j] = normalizedRect(rects[j], j);

                    realScore += individualRealScore(j);
                }
            }
        }
        bool bad = false;
        for (int j = 0; j < rects.size(); j++) {
            if (rects[j].area() == 0) {
                bad = true;
            }
            if (i != j) {
                bool X = overlap(rects[i].l, rects[i].r, rects[j].l, rects[j].r);
                bool Y = overlap(rects[i].d, rects[i].u, rects[j].d, rects[j].u);
                if (X && Y) {
                    bad = true;
                }
            }
        }
        if (bad) {
            realScore = -100000000;
        }


    }

    GeoRect normalizedRect(GeoRect geoRect, int i) {
        geoRect.l = max(0, geoRect.l);
        geoRect.d = max(0, geoRect.d);
        geoRect.r = min(10000, geoRect.r);
        geoRect.u = min(10000, geoRect.u);
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

inline GeoRect transform1(GeoRect rIdx, int &dir_dest) {
    int dir = rand() % 4;
    int x = random(-100, 100);
    if (dir == 0 || dir == 1) {
        if (dir == 0) {
            // 左伸ばす
            rIdx.l -= x;
            rIdx.r -= x;
        } else {
            rIdx.l += x;
            rIdx.r += x;
        }
    } else {
        if (dir == 2) {
            // した伸ばす
            rIdx.d -= x;
            rIdx.u -= x;
        } else {
            // うえ伸ばす
            rIdx.d += x;
            rIdx.u += x;
        }
    }
    dir_dest = dir;
    return rIdx;
}

inline GeoRect transform3(GeoRect rIdx, const Adv &adv, int &dir_dest) {
    double needArea = adv.r - rIdx.area();
    const int cap = 10;
    int dir = rand() % 4;
    if (dir == 0 || dir == 1) {
        int needLength = min(cap, ceil(needArea, (rIdx.u - rIdx.d)));
        if (dir == 0) {
            // 左伸ばす
            rIdx.l -= needLength;
        } else {
            // 右伸ばす
            rIdx.r += needLength;
        }
    } else {
        int needLength = min(cap, ceil(needArea, (rIdx.r - rIdx.l)));
        if (dir == 2) {
            // した伸ばす
            rIdx.d -= needLength;
        } else {
            // うえ伸ばす
            rIdx.u += needLength;
        }
    }
    dir_dest = dir;
    return rIdx;
}


class PhysicsSolver : Solver {
public:
    Output solve(Input input) {
        vector<GeoRect> rects;
        for (auto a : input.advs) {
            rects.emplace_back(a.p.x, a.p.x + 1, a.p.y, a.p.y + 1);
        }
        RectSet globalBest;
        globalBest.init(rects, input.advs);
        int iter = 0;

        auto attempt = [&](RectSet &rectSet, RectSet &bestRectSet, bool emit, double t) {
            const double currentScore = rectSet.score();
            iter++;
            auto remIndexes = removeIndexesWithTimer();
            bool force = false;
            if (remIndexes.size() > 0) {
                auto r = rectSet.rects;
                for (auto remi : remIndexes) {
                    auto q = input.advs[remi].p;
                    r[remi].l = q.x;
                    r[remi].r = q.x + 1;
                    r[remi].d = q.y;
                    r[remi].u = q.y + 1;
                }
                rectSet.init(r, input.advs);
                force = true;
            } else {
                int idx = rand() % input.n;
                GeoRect rIdx = rectSet.rects[idx];
                int rrr = rand() % 2;
                int dir = -1;

                if (rrr == 0) {
                    rIdx = transform1(rIdx, dir);
                } else {
                    rIdx = transform3(rIdx, input.advs[idx], dir);
                }
                rectSet.update(idx, rIdx, dir);
            }
            double nextScore = rectSet.score();
            double p = 1. * random() / RAND_MAX;
            bool ok = false;
            if (force || p < exp((nextScore - currentScore) / t)) {
                if (rectSet.realScore > bestRectSet.realScore) {
                    ok = bestRectSet.realScore;
                    bestRectSet = rectSet;
                }
                if (emit) {
                    emitJsonWithTimer(rectSet.rects, rectSet.realScore, input);
                }

            } else {
                rectSet.rollBack();
                ok = false;
            }
            return ok;
        };

        int attemptCnt = 0;
        while (!timer.is_TLE()) {
            if( attemptCnt > 3) break;
            double t = 0.01;
            RectSet rectSet;
            rectSet.init(rects, input.advs);
            RectSet best = rectSet;
            int fail = 0;
            while ( fail < 20000) {
                bool ok = attempt(rectSet, best, true, t);
                if (!ok) {
                    fail++;
                } else fail = 0;
                t *= 0.99997;
            }
            if (globalBest.strictScore() < best.strictScore()) {
                globalBest = best;
            }
            attemptCnt++;
        }

        // TODO: 明日へのTODO 当たり判定もしくは不正box修正アルゴリズムバグってない?
        // TODO: あとでかすぎるやつ検出する
        // TODO: かぶりを消すロジックがしょぼい
        // TODO: 絶対にたどり着けない頂点に対しては当たり判定チェックしない

        // TODO: 縮めるときに他を持ってくる? 伸ばすのと等価だね
        // TODO: プロダクションとテストでスコアが違うの、なんで?
        cerr << globalBest.realScore << " " << attemptCnt << endl;
        return createOutput(globalBest.rects, input);
    }
};

string itos(int n) {
    stringstream ss;
    ss << n;
    return ss.str();
}

int main() {
#ifdef CLION
    srand(time(NULL));
    const string communicationFile = "/tmp/" + itos(rand() % 100000) + ".com";
    registerCommunicationFile(communicationFile);
#endif
    srand(0);
#ifdef CLION
    auto inputSrc = loadFile("/home/kyuridenamida/ahc001/in/0002.txt");
    const Input input = Input::fromInputStream(inputSrc);
#else
    const Input input = Input::fromInputStream(cin);
#endif

//    input.outputToStream(cerr);
    auto sol = PhysicsSolver().solve(input);
    sol.output(cout);
//    cerr << sol.relativeScore() << endl;
}

