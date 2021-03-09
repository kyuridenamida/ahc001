#include <iostream>
#include <bitset>
#include <algorithm>
#include <complex>
#include "./common.h"
#include "visclient.h"
#include "./Timer.h"
using namespace std;

RealTimer timer(4.8);

typedef complex<double> GeoPoint;
typedef GeoPoint V;

int random(int a, int b) {
    return rand() % (b - a + 1) + a;
}

bool overlap(int a, int b, int A, int B) {
    if (min(b, B) - max(a, A) <= 0) {
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


double score(vector<GeoRect> rects, const Input &input) {
    double ans = 0;
    for (int i = 0; i < rects.size(); i++) {
        double h = 1 - 1. * min(rects[i].area(), input.advs[i].r) / max(rects[i].area(), input.advs[i].r);
        ans += 1 - h * h;
    }
    return ans / rects.size();
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

        auto q = input.advs[i].p;

        rects[i].l = min<int>(rects[i].l, q.x);
        rects[i].r = max<int>(rects[i].r, q.x + 1);
        rects[i].d = min<int>(rects[i].d, q.y);
        rects[i].u = max<int>(rects[i].u, q.y + 1);
    }

    for (int i = 0; i < rects.size(); i++) {
        if (rects[i].l > rects[i].r)
            swap(rects[i].l, rects[i].r);
        if (rects[i].d > rects[i].u)
            swap(rects[i].u, rects[i].d);
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
        double h = 1 - 1. *  min(r.area(), input.advs[idx].r) / max(r.area(), input.advs[idx].r);
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

class PhysicsSolver : Solver {
public:
    Output solve(Input input) {
//        reset();
        vector<GeoRect> rects;
        for (auto a : input.advs) {
            rects.emplace_back(a.p.x, a.p.x + 1, a.p.y, a.p.y + 1);
        }

        double currentScore = score(rects, input);
        double bestScore = currentScore;
        vector<GeoRect> bestRects;

        double t = 0.00001;
        while (!timer.is_TLE()) {
            int cap = bestScore > 0.85 ? 10 : 100;
            vector<GeoRect> r = rects;
            int idx = rand() % input.n;
            int dir = rand() % 4;

            double needArea = input.advs[idx].r - r[idx].area();

            int rrr = rand() % 2;
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
            }

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

            r = fixit(r, input, idx, dir);
            if (r.size() == 0) continue;

            double nextScore = score(r, input);

//            if( nextScore >= currentScore){
            double p = 1. * random() / RAND_MAX;
            if (p < exp((nextScore - currentScore) / t)) {
//                cerr << currentScore << endl;
                currentScore = nextScore;
                rects = r;
                if (currentScore > bestScore) {
                    bestScore = currentScore;
                    bestRects = rects;
                    emitJson(createJson(r, currentScore, input));
                }

            }
//            t *= 0.99997;
        }
        return createOutput(bestRects, input);
    }
};


int main() {
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
