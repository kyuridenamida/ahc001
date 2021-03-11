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


double realScore(vector<GeoRect> rects, const Input &input) {
    double ans = 0;
    for (int i = 0; i < rects.size(); i++) {
        double h = 1 - 1. * min(rects[i].area(), input.advs[i].r) / max(rects[i].area(), input.advs[i].r);
        ans += 1 - h * h;
    }
    return ans / rects.size();
}

double score(vector<GeoRect> rects, const Input &input, bool penSwitch) {
    double ans = 0;
    double penalty = 0;
    for (int i = 0; i < rects.size(); i++) {
        double h = 1 - 1. * min(rects[i].area(), input.advs[i].r) / max(rects[i].area(), input.advs[i].r);
        ans += (1 - h * h);

    }
//    for (int i = 0; i < rects.size(); i++) {
//        for (int j = i + 1; j < rects.size(); j++) {
//            if (i != j) {
//                int X = overlap(rects[i].l, rects[i].r, rects[j].l, rects[j].r);
//                int Y = overlap(rects[i].d, rects[i].u, rects[j].d, rects[j].u);
//                penalty += 1.0 * (X * Y) / min(rects[i].area(), rects[j].area());
//            }
//        }
//    }
    return ans / rects.size() - penalty * 1000;
}

vector<GeoRect> fixit(vector<GeoRect> rects, const Input &input, int targetIndex, int dir) {

    for (int j = 0; j < rects.size(); j++) {
        if (targetIndex != j) {
            bool X = overlap(rects[targetIndex].l, rects[targetIndex].r, rects[j].l, rects[j].r);
            bool Y = overlap(rects[targetIndex].d, rects[targetIndex].u, rects[j].d, rects[j].u);
            if (X && Y) {
                if (dir == 0) {
                    rects[j].r = rects[targetIndex].l;
                } else if (dir == 1) {
                    rects[j].l = rects[targetIndex].r;
                } else if (dir == 2) {
                    rects[j].u = rects[targetIndex].d;
                } else if (dir == 3) {
                    rects[j].d = rects[targetIndex].u;
                }
            }
        }
    }
    for (int i = 0; i < rects.size(); i++) {
        rects[i].l = max(0, rects[i].l);
        rects[i].d = max(0, rects[i].d);
        rects[i].r = min(10000, rects[i].r);
        rects[i].u = min(10000, rects[i].u);
    }

    for (int i = 0; i < rects.size(); i++) {
        auto q = input.advs[i].p;
        rects[i].l = min<int>(rects[i].l, q.x);
        rects[i].r = max<int>(rects[i].r, q.x + 1);
        rects[i].d = min<int>(rects[i].d, q.y);
        rects[i].u = max<int>(rects[i].u, q.y + 1);
    }

//    for (int j = 0; j < rects.size(); j++) {
//        if (rects[j].area() == 0) {
//            return {};
//        }
//        if (targetIndex != j) {
//            bool X = overlap(rects[targetIndex].l, rects[targetIndex].r, rects[j].l, rects[j].r);
//            bool Y = overlap(rects[targetIndex].d, rects[targetIndex].u, rects[j].d, rects[j].u);
//            if (X && Y) {
//                return {};
//            }
//        }
//    }

    return rects;
}

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
    globalCommunicationFile = communicationFile;
}

vector<int> removeIndexes() {
    registerCommunicationFile(globalCommunicationFile);
    ifstream ifs(globalCommunicationFile);
    if (!ifs.is_open()) {
        return {};
    };
    vector<int> indexes;
    int idx;
    while (ifs >> idx) {
        indexes.push_back(idx);
    }
    ifs.close();

    ofstream ofs(globalCommunicationFile);
    ofs.close();

    return indexes;
}

double _lstsub2 = -1;

vector<int> removeIndexesWithTimer() {
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
        _lstsub = now;
    }


}

class PhysicsSolver : Solver {
public:
    Output solve(Input input) {

//        reset();
        vector<GeoRect> rects;
        for (auto a : input.advs) {
            rects.emplace_back(a.p.x, a.p.x + 1, a.p.y, a.p.y + 1);
        }

        double currentScore = score(rects, input, true);
        double bestScore = realScore(rects, input);
        vector<GeoRect> bestRects;

        double t = 0.0001;
        int iter = 0;
        while (!timer.is_TLE()) {
            iter++;
//            t = max(t, 0.0001);
            int cap = 100;
            bool force = false;
            vector<GeoRect> r = rects;
            int idx = rand() % input.n;
            for (int j = 0; j < 1; j++) {
                int dir = rand() % 4;

                double needArea = input.advs[idx].r - r[idx].area();
                int rrr = rand() % 3;
                if (rrr == 0) {
                    int x = random(1, 100);
                    if (dir == 0 || dir == 1) {
                        if (dir == 0) {
                            // 左伸ばす
                            r[idx].l -= x;
                            r[idx].r -= x;
                        } else {
                            r[idx].l += x;
                            r[idx].r += x;
                        }
                    } else {
                        if (dir == 2) {
                            // した伸ばす
                            r[idx].d -= x;
                            r[idx].u -= x;
                        } else {
                            // うえ伸ばす
                            r[idx].d += x;
                            r[idx].u += x;
                        }
                    }

                } else if (rrr == 1) {
                    if (dir == 0 || dir == 1) {
                        int needLength = random(1, 100);
                        if (dir == 0) {
                            // 左伸ばす
                            r[idx].l -= needLength;
                        } else {
                            // 右伸ばす
                            r[idx].r += needLength;
                        }
                    } else {
                        int needLength = random(1, 100);
                        if (dir == 2) {
                            // した伸ばす
                            r[idx].d -= needLength;
                        } else {
                            // うえ伸ばす
                            r[idx].u += needLength;
                        }
                    }
                }
                else {
                    if (dir == 0 || dir == 1) {
                        int needLength = min(cap, ceil(needArea, (r[idx].u - r[idx].d)));
                        if (dir == 0) {
                            // 左伸ばす
                            r[idx].l -= needLength;
                        } else {
                            // 右伸ばす
                            r[idx].r += needLength;
                        }
                    } else {
                        int needLength = min(cap, ceil(needArea, (r[idx].r - r[idx].l)));
                        if (dir == 2) {
                            // した伸ばす
                            r[idx].d -= needLength;
                        } else {
                            // うえ伸ばす
                            r[idx].u += needLength;
                        }
                    }
                }
                r = fixit(r, input, idx, dir);
                if (r.size() == 0) break;
            }


            if (r.size() == 0) continue;

            auto remIndexes = removeIndexesWithTimer();
            for (auto remi : remIndexes) {
                auto q = input.advs[remi].p;
                r[remi].l = q.x;
                r[remi].r = q.x + 1;
                r[remi].d = q.y;
                r[remi].u = q.y + 1;
                force = true;
            }
            r = fixit(r, input, idx, 0);

            double nextScore = score(r, input, timer.relative_time_elapsed() < 0.8);

//            if( nextScore >= currentScore){
            double p = 1. * random() / RAND_MAX;
            if (force || p < exp((nextScore - currentScore) / t)) {
                currentScore = nextScore;
                rects = r;

                double real = realScore(r, input);
                if (real > bestScore) {
                    bestScore = real;
                    bestRects = rects;
                }
            }
            t *= 0.99997;
        }
        // TODO: 明日へのTODO 当たり判定もしくは不正box修正アルゴリズムバグってない?
        // TODO: あとでかすぎるやつ検出する
        // TODO: かぶりを消すロジックがしょぼい
        // TODO: 絶対にたどり着けない頂点に対しては当たり判定チェックしない
        cout << iter << endl;
        return createOutput(bestRects, input);
    }
};

string itos(int n) {
    stringstream ss;
    ss << n;
    return ss.str();
}

int main() {
    srand(time(NULL));
    const string communicationFile = "/tmp/" + itos(rand() % 100000) + ".com";
    registerCommunicationFile(communicationFile);
    srand(10);
#ifdef CLION
    auto inputSrc = loadFile("/home/kyuridenamida/ahc001/in/0098.txt");
    const Input input = Input::fromInputStream(inputSrc);
#else
    const Input input = Input::fromInputStream(cin);
#endif

    auto sol = PhysicsSolver().solve(input);
//    sol.output(cout);
}

