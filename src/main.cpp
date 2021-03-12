
#include <iostream>
#include <bitset>
#include <algorithm>
#include <complex>
#include "./common.h"
#include "visclient.h"
#include "./Timer.h"
#include "./xorshift.h"

using namespace std;

XorShift xorShift;

RealTimer timer(5.0);

typedef complex<double> GeoPoint;
typedef GeoPoint V;


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

    GeoRect() { }

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
                        rects[j] = GeoRect(rects[j].l, geoRect_.l, rects[j].d, rects[j].u);
                    } else if (dir == 1) {
                        rects[j] = GeoRect(geoRect_.r, rects[j].r, rects[j].d, rects[j].u);
                    } else if (dir == 2) {
                        rects[j] = GeoRect(rects[j].l, rects[j].r, rects[j].d, geoRect_.d);
                    } else if (dir == 3) {
                        rects[j] = GeoRect(rects[j].l, rects[j].r,  geoRect_.u, rects[j].u);
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
    int dir = xorShift.next_uint32(0, 4);
    int x = xorShift.next_uint32(-100, 100);
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
    int dir = xorShift.next_uint32(0, 4);
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
                int idx = xorShift.next_uint32(0, input.n);
                GeoRect rIdx = rectSet.rects[idx];
                int rrr = xorShift.next_uint32(0, 2);
                int dir = -1;

                if (rrr == 0) {
                    rIdx = transform1(rIdx, dir);
                } else {
                    rIdx = transform3(rIdx, input.advs[idx], dir);
                }
                rectSet.update(idx, rIdx, dir);
            }
            double nextScore = rectSet.score();
            double p = xorShift.next_prob();
            bool ok = false;
            if (force || p < exp((nextScore - currentScore) / t)) {
//            if (nextScore > currentScore) {
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

        while (!timer.is_TLE()) {
            double t = 0.01;
            RectSet rectSet;
            rectSet.init(rects, input.advs);
            RectSet best = rectSet;
            int fail = 0;
            while (fail < 20000) {
                bool ok = attempt(rectSet, best, true, t);
                if (!ok) {
                    fail++;
                } else fail = 0;
                t *= 0.99997;
            }
            if (globalBest.score() < best.score()) {
                globalBest = best;
            }
        }

        // TODO: 明日へのTODO 当たり判定もしくは不正box修正アルゴリズムバグってない?
        // TODO: あとでかすぎるやつ検出する
        // TODO: かぶりを消すロジックがしょぼい
        // TODO: 絶対にたどり着けない頂点に対しては当たり判定チェックしない

        // TODO: 縮めるときに他を持ってくる? 伸ばすのと等価だね
        // TODO: プロダクションとテストでスコアが違うの、なんで?
        // sanitizerまわりぽいなあ
        cerr << globalBest.realScore << endl;
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
