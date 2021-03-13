#include <iostream>
#include <bitset>
#include <algorithm>
#include <complex>
#include <set>
#include <random>
#include <utility>
#include "./common.h"
#include "./httputils.h"
#include "./Timer.h"
#include "./xorshift.h"
#include "visualizer.h"

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
        cerr << "Read " << removeIndexes.size() << " delete removeIndexes" << endl;

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

// Utils

bool overlap(int a, int b, int A, int B) {
    if (B <= a || b <= A) {
        return false;
    }
    return true;
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

    Rect toRect() {
        return Rect(P((int) l, (int) d), Size((int) (r - l), (int) (u - d)));
    }
};

double _lstsub = -1;


string buildReportJson(vector<GeoRect> rects, double score, double fakeScore, Input input) {
    using namespace HttpUtils;
    auto f = [&](GeoRect r, int idx) {
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
    auto log =  mapToJson(
            {
                    {"rects",     rectsArray.str()},
                    {"type",      jsonValue("draw")},
                    {"fakeScore", jsonValue(fakeScore)},
                    {"score",     jsonValue(score)},
            }
    );
    cerr << log << endl;
    return log;
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


    void update(const int i, const GeoRect &geoRect_, int dir, int diff) {
        if (diff == 0)
            return;
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
                    if (dir == 0) {
                        op = GeoRect(op.l, geoRect_.l, op.d, op.u);
                    } else if (dir == 1) {
                        op = GeoRect(geoRect_.r, op.r, op.d, op.u);
                    } else if (dir == 2) {
                        op = GeoRect(op.l, op.r, op.d, geoRect_.d);
                    } else if (dir == 3) {
                        op = GeoRect(op.l, op.r, geoRect_.u, op.u);
                    } else if (dir == 4) {
                        op = GeoRect(op.l, geoRect_.l, op.d, geoRect_.d);
                    } else if (dir == 5) {
                        op = GeoRect(geoRect_.r, op.r, geoRect_.u, op.u);
                    } else if (dir == 6) {
                        op = GeoRect(op.l, geoRect_.l, geoRect_.u, op.u);
                    } else if (dir == 7) {
                        op = GeoRect(geoRect_.r, op.r, op.d, geoRect_.d);
                    }
                    op = normalizedRect(op, j);

                    realScore += individualRealScore(j);

                    bool X = overlap(rects[i].l, rects[i].r, rects[j].l, rects[j].r);
                    bool Y = overlap(rects[i].d, rects[i].u, rects[j].d, rects[j].u);
                    if (X && Y) {
                        bad = true;
                        break;
                    }
                }
            }
        }
        if (!bad) {
            for (int j = 0; j < n; j++) {
                if (i != j) {
                    bool X = overlap(rects[i].l, rects[i].r, rects[j].l, rects[j].r);
                    bool Y = overlap(rects[i].d, rects[i].u, rects[j].d, rects[j].u);
                    if (X && Y) {
                        bad = true;
                        break;
                    }
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
    int dir = ctx->rng->next_uint32(0, 4);
    int x = ctx->rng->next_uint32(-100, 100);
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

inline GeoRect transform2(GeoRect rIdx, int &dir_dest, int &diff) {
    int dir = ctx->rng->next_uint32(0, 4);
    int x = ctx->rng->next_uint32(-100, 100);
    if (dir == 0 || dir == 1) {
        if (dir == 0) {
            // 左伸ばす
            rIdx.l -= x;
        } else {
            rIdx.r += x;
        }
    } else {
        if (dir == 2) {
            // した伸ばす
            rIdx.d -= x;
        } else {
            // うえ伸ばす
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
    int dir = ctx->rng->next_uint32(0, 8);
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
        int needLength = ctx->rng->next_uint32(1, 10);
        rIdx.l -= needLength;
        rIdx.d -= needLength;
        diff = needLength;
    } else if (dir == 5) {
        // 右上のばす
        int needLength = ctx->rng->next_uint32(1, 10);
        rIdx.r += needLength;
        rIdx.u += needLength;
        diff = needLength;
    } else if (dir == 6) {
        // 左上のばす
        int needLength = ctx->rng->next_uint32(1, 10);
        rIdx.l -= needLength;
        rIdx.u += needLength;
        diff = needLength;
    } else if (dir == 7) {
        // 右下のばす
        int needLength = ctx->rng->next_uint32(1, 10);
        rIdx.r += needLength;
        rIdx.d -= needLength;
        diff = needLength;
    }
    dir_dest = dir;
    return rIdx;
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


class SimulatedAnnealingSolver {
public:
    SimulatedAnnealingSolver() {}

public:

    Output solve(Input input, const Args &args) {
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
            auto resp = ctx->visCom->receiveResponseIfExists();
            bool force = false;
            if (resp.received) {
                auto r = rectSet.rects;
                for (auto remi : resp.removeIndexes) {
                    auto q = input.advs[remi].p;
                    r[remi].l = q.x;
                    r[remi].r = q.x + 1;
                    r[remi].d = q.y;
                    r[remi].u = q.y + 1;
                }
                rectSet.init(r, input.advs);
                force = true;
                forceIdx = resp.removeIndexes;
            } else {
                int idx =
                        !forceIdx.empty() ? forceIdx[ctx->rng->next_uint32(0, forceIdx.size())]
                                          : ctx->rng->next_uint32(0, input.n);

                GeoRect rIdx = rectSet.rects[idx];
                int rrr = ctx->rng->next_uint32(0, 3);
                int dir = -1;
                int diff = -1;
                if (rrr == 0) {
                    rIdx = transform1(rIdx, dir, diff);
                } else if (rrr == 1) {
                    rIdx = transform2(rIdx, dir, diff);
                } else if (rrr == 2) {

                    rIdx = transform3(rIdx, input.advs[idx], dir, diff);
                }
                rectSet.update(idx, rIdx, dir, diff);

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
                    ctx->vis->emitJsonWithTimer(jsonBuilder);
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
        return createOutput(globalBest, input);
    }

private:
    static Output createOutput(RectSet rects, const Input &input) {
        vector<OutputItem> res;
        for (auto i : input.advs) {
            res.emplace_back(i, rects.rects[i.id].toRect());
        }
        return Output(input, res);
    }
};

class Main {
public:
    static void run(Args args, istream &is) {
        auto *timer = new RealTimer(TIME_LIMIT_SECONDS);
        auto *rng = new XorShift();
        auto *vis = new Visualizer(timer);
        AHC001VisualizerCommunicator *visCom = AHC001VisualizerCommunicator::start(vis, timer);

        registerApplicationContext(new ApplicationContext(timer, rng, vis, visCom));

        Output sol = SimulatedAnnealingSolver().solve(Input::fromInputStream(is), args);
        sol.output(cout);
    }

};

int main(int argc, char *argv[]) {
    Args args = Args::fromProgramArgs(argc, argv);
#ifdef CLION
    auto inputSrc = loadFile("/home/kyuridenamida/ahc001/in/0098.txt");
    Main::run(args, inputSrc);
#else
    Main::run(args, cin);
#endif
}
