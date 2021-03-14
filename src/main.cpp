#include <iostream>
#include <bitset>
#include <algorithm>
#include <complex>
#include <set>
#include <random>
#include "./common.h"
#include "./timer.h"
#include "./xorshift.h"
#include "./visualizer.h"

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
        prevItems.clear();
        prevItems.emplace_back(i, rects[i]);
        prevRealScore = realScore;
        rollbackable = true;
        if (pushLength == 0)
            return true;
        auto geoRect = normalizedRect(geoRect_, i);

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
//                            cerr << (geoRect != geoRect_) << ")" << endl;
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


string buildReportJson(RectSet rectSet, double score, double fakeScore, Input input, double T) {
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
    for (int i = 0; i < rectSet.rects.size(); i++) {
        if (i != 0) {
            rectsArray << ",";
        }
        rectsArray << f(rectSet.rects[i], i);
    }
    rectsArray << "]";
    return mapToJson(
            {
                    {"rects",       rectsArray.str()},
                    {"type",        jsonValue("draw")},
                    {"fakeScore",   jsonValue(fakeScore)},
                    {"score",       jsonValue(score)},
                    {"relTime",     jsonValue(ctx->timer->relative_time_elapsed())},
                    {"temperature", jsonValue(T)},

            }
    );
}

inline Rect shake(Rect rIdx, DIR &dir_dest, int &pushLength) {
    DIR dir = static_cast<DIR>(ctx->rng->next_uint32(0, 4));
    int moveAmount = ctx->rng->next_uint32(1, 100);
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
    int pushLength = ctx->rng->next_uint32(-200, 200);
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
    static const int EXPECTED_PARAM_COUNT = 5;
    double saStartTemp = 0.0005;
    double saEndTemp = 0.000001;
    int randomSeed = 0;
    double paramsA = 1;
    double paramsB = 4;

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
        ssForParsing >> args.saStartTemp;
        ssForParsing >> args.saEndTemp;
        ssForParsing >> args.randomSeed;
        ssForParsing >> args.paramsA;
        ssForParsing >> args.paramsB;
        return args;
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
                    return buildReportJson(rectSet, rectSet.getRealScore(), rectSet.score(), input, t);
                };
                ctx->vis->emitJsonWithTimer(jsonBuilder);
            }
        } else {
            rectSet.rollBack();
            ok = false;
        }
        return ok;
    };

    RectSet rectSet;
    rectSet.init(rects, input.advs);

    auto computeTemperature = [&](double progress) {
        if( progress > M_PI / 5 ){
            return 0.00001 * (1-progress);
        }
        return args.saStartTemp - args.saStartTemp * sin(progress * 2.5 * M_PI) * sin(progress * 2.5 * M_PI);
    };

    while (!ctx->timer->is_TLE()) {
        double T = computeTemperature(ctx->timer->relative_time_elapsed());
        attempt(rectSet, globalBest, true, T);
    }
//    cout << iter << " " << globalBest.score() << endl;
    return Output(globalBest.rects);
}

void runMain(Args args, istream &is) {
    auto *timer = new RealTimer(TIME_LIMIT_SECONDS);
    auto *rng = new XorShift();
    auto *vis = new Visualizer(timer);
    vis->emitJson(HttpUtils::mapToJson(
            {
                    {"type", HttpUtils::jsonValue("reset")}
            }
    ));
    AHC001VisualizerCommunicator *visCom = AHC001VisualizerCommunicator::start(vis, timer);

    registerApplicationContext(new ApplicationContext(timer, rng, vis, visCom));

    Output solution = solveBySimulatedAnnealing(Input::fromInputStream(is), args);
    solution.output(cout);
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
