#include <iostream>
#include <bitset>
#include <algorithm>
#include <complex>
#include "./common.h"
#include "visclient.h"
#include "./Timer.h"

using namespace std;

RealTimer timer(120);

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

double score(vector<GeoRect> rects, const Input &input) {
    double ans = 0;
    for (int i = 0; i < rects.size(); i++) {
        double h = 1 - 1. * min(rects[i].area(), input.advs[i].r) / max(rects[i].area(), input.advs[i].r);
        ans += 1 - h * h;
    }
    return ans / rects.size();
}

vector<GeoRect> fixit(vector<GeoRect> rects, const Input &input, int targetIndex) {
    for (int j = 0; j < rects.size(); j++) {
        if (targetIndex != j) {
            bool X = overlap(rects[targetIndex].l, rects[targetIndex].r, rects[j].l, rects[j].r);
            bool Y = overlap(rects[targetIndex].d, rects[targetIndex].u, rects[j].d, rects[j].u);
            if (X && Y) {
                int r = rand() % 3 + 1;
                if (r & 1) {
                    if (rects[targetIndex].l < rects[j].l) {
                        swap(rects[targetIndex].r, rects[j].l);
                    } else {
                        swap(rects[targetIndex].l, rects[j].r);
                    }
                }
                if (r & 2) {
                    if (rects[targetIndex].d < rects[j].d) {
                        swap(rects[targetIndex].u, rects[j].d);
                    } else {
                        swap(rects[targetIndex].d, rects[j].u);
                    }
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
        if (rects[i].l > rects[i].r)
            swap(rects[i].l, rects[i].r);
        if (rects[i].d > rects[i].u)
            swap(rects[i].u, rects[i].d);
    }
    for (int i = 0; i < rects.size(); i++) {
        auto q = input.advs[i].p;
        rects[i].l = min<int>(rects[i].l, q.x);
        rects[i].r = max<int>(rects[i].r, q.x + 1);
        rects[i].d = min<int>(rects[i].d, q.y);
        rects[i].u = max<int>(rects[i].u, q.y + 1);
    }

    for (int j = 0; j < rects.size(); j++) {
        if (rects[j].area() == 0) {
            return {};
        }
        if (targetIndex != j) {
            bool X = overlap(rects[targetIndex].l, rects[targetIndex].r, rects[j].l, rects[j].r);
            bool Y = overlap(rects[targetIndex].d, rects[targetIndex].u, rects[j].d, rects[j].u);
            if (X && Y) {
                return {};
            }
        }
    }

    return rects;
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
    ss << quoted("score") << ": " << score << "\n";
    ss << "}";
    return ss.str();
}

bool isValid(vector<GeoRect> rects) {
    for (int targetIndex = 0; targetIndex < rects.size(); targetIndex++) {
        if (rects[targetIndex].area() == 0) {
            return false;
        }
        for (int j = 0; j < rects.size(); j++) {

            if (targetIndex != j) {
                bool X = overlap(rects[targetIndex].l, rects[targetIndex].r, rects[j].l, rects[j].r);
                bool Y = overlap(rects[targetIndex].d, rects[targetIndex].u, rects[j].d, rects[j].u);
                if (X && Y) {
                    return false;
                }
            }
        }
    }
    return true;
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

class PhysicsSolver : Solver {
public:
    Output solve(Input input) {

//        reset();
        vector<GeoRect> rects;
        for (auto a : input.advs) {
            rects.emplace_back(a.p.x, a.p.x + 1, a.p.y, a.p.y + 1);
        }

        double currentScore = score(rects, input);
        double bestScore = realScore(rects, input);
        vector<GeoRect> bestRects;

        double t = 1000;
        while (!timer.is_TLE()) {
            int cap = 100;
            vector<GeoRect> r = rects;
            int idx = rand() % input.n;
            int dir = rand() % 4;

            double needArea = input.advs[idx].r - r[idx].area();
            needArea *= 0.1;
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
                    int needLength = random(1, 20);
                    if (dir == 0) {
                        // 左伸ばす
                        r[idx].l -= needLength;
                    } else {
                        // 右伸ばす
                        r[idx].r += needLength;
                    }
                } else {
                    int needLength = random(1, 20);
                    if (dir == 2) {
                        // した伸ばす
                        r[idx].d -= needLength;
                    } else {
                        // うえ伸ばす
                        r[idx].u += needLength;
                    }
                }
            } else {
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

            r = fixit(r, input, idx);
            if( r.size() == 0 ){
                continue;
            }

            double nextScore = score(r, input);

//            if( nextScore >= currentScore){
            double p = 1. * random() / RAND_MAX;
            if (p < exp((nextScore - currentScore) / t)) {
                cerr << currentScore << endl;
                currentScore = nextScore;
                rects = r;

                double real = realScore(r, input);
                if (real > bestScore) {
                    bestScore = real;
                    bestRects = rects;
                }
                cerr << currentScore << " " << real << endl;
                emitJsonWithTimer(r, real, input);

            }
            t *= 0.99997;
        }
        // TODO: 明日へのTODO 当たり判定もしくは不正box修正アルゴリズムバグってない?
        // TODO: あとでかすぎるやつ検出する
        // TODO: かぶりを消すロジックがしょぼい
        return
                createOutput(bestRects, input
                );
    }

};


int main() {
    srand(30);
#ifdef CLION
    auto inputSrc = loadFile("/home/kyuridenamida/ahc001/input/seed0.txt");
    const Input input = Input::fromInputStream(inputSrc);
#else
    const Input input = Input::fromInputStream(cin);
#endif

//    input.outputToStream(cerr);
    auto sol = PhysicsSolver().solve(input);
    sol.output(cout);
//    cerr << sol.relativeScore() << endl;
}
