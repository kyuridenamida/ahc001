
#include <iostream>
#include <bitset>
#include <algorithm>
#include <complex>
#include <set>
#include "./common.h"
#include "visclient.h"
#include "./Timer.h"
#include "./xorshift.h"

using namespace std;

XorShift xorShift;

RealTimer timer(5);

typedef complex<double> GeoPoint;
typedef GeoPoint V;


bool overlap(int a, int b, int A, int B) {
    if (B <= a || b <= A) {
        return false;
    }
    return true;
}

int overlapLength(int a, int b, int A, int B) {
    return max(0, min(B, b) - max(a, A));
}

inline bool eq(int a, int b) {
    return a == b;
}


int ceil(int a, int b) {
    return (a + b - 1) / b;
}

class GeoRect {

public:
    int l;
    int r;
    int d;
    int u;

    GeoRect() {}

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

    GeoRect expand(int dir, const Adv &adv) {
        if (l == r || u == d) {
            return *this;
        }
        GeoRect rIdx = *this;
        double needArea = adv.r - area();
        const int cap = 10;
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
        return rIdx;
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
        cerr << idx << " ";
    }
    cerr << endl;
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

string createJson(vector<GeoRect> rects, double score, double fakeScore, Input input) {
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
    ss << quoted("fakeScore") << ":" << fakeScore << ",\n";
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


double _lstsub = -1;

void emitJsonWithTimer(vector<GeoRect> rects, double realScore, double fakeScore, Input input) {
    double now = timer.time_elapsed();
    if (now - _lstsub > 0.016) {
        emitJson(createJson(rects, realScore, fakeScore, input));
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
private:
    double realScore;
public:
    int n;
    vector<GeoRect> rects;
    vector<Adv> advs;


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


    void update(int i, const GeoRect &geoRect_, int dir, int diff) {
        if( diff == 0 )
            return;
        auto geoRect = normalizedRect(geoRect_, i);
        prevItems.clear();
        prevItems.emplace_back(i, rects[i]);
        prevRealScore = realScore;
        rollbackable = true;

        realScore -= individualRealScore(i);
        rects[i] = geoRect;
        realScore += individualRealScore(i);

        for (int j = 0; j < n; j++) {
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
                        rects[j] = GeoRect(rects[j].l, rects[j].r, geoRect_.u, rects[j].u);
                    } else if (dir == 4) {
                        rects[j] = GeoRect(rects[j].l, geoRect_.l, rects[j].d, geoRect_.d);
                    } else if (dir == 5) {
                        rects[j] = GeoRect(geoRect_.r, rects[j].r, geoRect_.u, rects[j].u);
                    } else if (dir == 6) {
                        rects[j] = GeoRect(rects[j].l, geoRect_.l, geoRect_.u, rects[j].u);
                    } else if (dir == 7) {
                        rects[j] = GeoRect(geoRect_.r, rects[j].r, rects[j].d, geoRect_.d);
                    }
                    rects[j] = normalizedRect(rects[j], j);

                    realScore += individualRealScore(j);
                }
            }
        }
        bool bad = false;
        for (int j = 0; j < n; j++) {
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

inline GeoRect transform1(GeoRect rIdx, int &dir_dest, int &diff) {
    int dir = xorShift.next_uint32(0, 4);
    int x = xorShift.next_uint32(-10, 100);
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
    diff = x;
    dir_dest = dir;
    return rIdx;
}

inline GeoRect transform3(GeoRect rIdx, const Adv &adv, int &dir_dest, int diff) {
    double needArea = adv.r - rIdx.area();
    const int cap = 10;
    int dir = xorShift.next_uint32(0, 8);
    if (dir == 0 || dir == 1) {
        int needLength = min(cap, ceil(needArea, (rIdx.u - rIdx.d)));
        if (dir == 0) {
            // 左伸ばす
            rIdx.l -= needLength;
        } else {
            // 右伸ばす
            rIdx.r += needLength;
        }
        diff = needLength;
    } else if (dir == 2 || dir == 3) {
        int needLength = min(cap, ceil(needArea, (rIdx.r - rIdx.l)));
        if (dir == 2) {
            // した伸ばす
            rIdx.d -= needLength;
        } else {
            // うえ伸ばす
            rIdx.u += needLength;
        }
        diff = needLength;
    } else if (dir == 4) {
        // 左したのばす
        int needLength = xorShift.next_uint32(1, 10);
        rIdx.l -= needLength;
        rIdx.d -= needLength;
        diff = needLength;
    } else if (dir == 5) {
        // 右上のばす
        int needLength = xorShift.next_uint32(1, 10);
        rIdx.r += needLength;
        rIdx.u += needLength;
        diff = needLength;
    } else if (dir == 6) {
        // 左上のばす
        int needLength = xorShift.next_uint32(1, 10);
        rIdx.l -= needLength;
        rIdx.u += needLength;
        diff = needLength;
    } else if (dir == 7) {
        // 右下のばす
        int needLength = xorShift.next_uint32(1, 10);
        rIdx.r += needLength;
        rIdx.d -= needLength;
        diff = needLength;
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

        vector<int> forceIdx = {};
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
                forceIdx = remIndexes;
            } else {
                int idx =
                        forceIdx.size() > 0 ? forceIdx[xorShift.next_uint32(0, forceIdx.size())] : xorShift.next_uint32(
                                0, input.n);
//                vector< pair<double,int> > values;
//                if( timer.time_elapsed() < 30.0) {
//                    for (int i = 0; i < rectSet.n; i++) {
//                        values.emplace_back(rectSet.individualRealScore(i), i);
//                    }
//                    sort(values.begin(), values.end());
//                    values.resize(5);
//                    idx = values[xorShift.next_uint32(values.size())].second;
//                }

                GeoRect rIdx = rectSet.rects[idx];
                int rrr = xorShift.next_uint32(0, 2);
                int dir = -1;
                int diff = -1;
                if (rrr == 0) {
                    rIdx = transform1(rIdx, dir, diff);
                } else {
                    rIdx = transform3(rIdx, input.advs[idx], dir, diff);
                }
                rectSet.update(idx, rIdx, dir, diff);

            }
            double nextScore = rectSet.score();
            double p = xorShift.next_prob();
            bool ok = false;
            if (nextScore > 0 && (force || p < exp((nextScore - currentScore) / t))) {
//            if (nextScore > currentScore) {
                if (rectSet.getRealScore() > bestRectSet.getRealScore()) {
                    ok = true;
                    bestRectSet = rectSet;
                }
                if (emit) {
                    emitJsonWithTimer(rectSet.rects, rectSet.getRealScore(), rectSet.score(), input);
                }

            } else {
                rectSet.rollBack();
                ok = false;
            }
            return ok;
        };

        double start_temp = 0.001;
        double end_temp = 0.000001;

        RectSet rectSet;
        rectSet.init(rects, input.advs);
        while (!timer.is_TLE()) {
            double temp =
                    start_temp +
                    timer.relative_time_elapsed() * (end_temp - start_temp);
            attempt(rectSet, globalBest, true, temp);


        }
        //　TODO: 高速化 check herasu


        // TODO: 明日へのTODO 当たり判定もしくは不正box修正アルゴリズムバグってない?
        // TODO: あとでかすぎるやつ検出する
        // TODO: かぶりを消すロジックがしょぼい
        // TODO: 絶対にたどり着けない頂点に対しては当たり判定チェックしない

        // TODO: 縮めるときに他を持ってくる? 伸ばすのと等価だね
        // TODO: プロダクションとテストでスコアが違うの、なんで?
        // sanitizerまわりぽいなあ
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
    auto inputSrc = loadFile("/home/kyuridenamida/ahc001/in/0098.txt");
    const Input input = Input::fromInputStream(inputSrc);
#else
    const Input input = Input::fromInputStream(cin);
#endif

//    input.outputToStream(cerr);
    auto sol = PhysicsSolver().solve(input);
    sol.output(cout);
//    cerr << sol.relativeScore() << endl;
}
