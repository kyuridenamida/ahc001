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

inline Rect extend(Rect rIdx, DIR &dir_dest, int &diff) {
    DIR dir = static_cast<DIR>(ctx->rng->next_uint32(0, 4));
    int pushLength = ctx->rng->next_uint32(-100, 100);
    if (dir == DIR::LEFT || dir == DIR::RIGHT) {
        if (dir == DIR::LEFT) {
            // 左伸ばす
            rIdx.l -= pushLength;
        } else {
            rIdx.r += pushLength;
        }
    } else {
        if (dir == DIR::DOWN) {
            // した伸ばす
            rIdx.d -= pushLength;
        } else {
            // うえ伸ばす
            rIdx.u += pushLength;
        }
    }
    diff = pushLength;
    dir_dest = dir;
    return rIdx;
}

inline Rect stickyExtend(Rect rIdx, const Adv &adv, DIR &dir_dest, int &pushLength) {
    double needArea = adv.r - rIdx.area();
    const int cap = 10;
    DIR dir = static_cast<DIR>(ctx->rng->next_uint32(0, 8));

    int needLength = 1e9;
    if (dir == DIR::LEFT || dir == DIR::RIGHT) {
        needLength = min(cap, ceil(needArea, (rIdx.u - rIdx.d)));
        if (dir == DIR::LEFT) {
            // 左伸ばす
            rIdx.l -= needLength;
        } else {
            // 右伸ばす
            rIdx.r += needLength;
        }
    } else if (dir == DIR::DOWN || dir == DIR::UP) {
        needLength = min(cap, ceil(needArea, (rIdx.r - rIdx.l)));
        if (dir == DIR::DOWN) {
            // した伸ばす
            rIdx.d -= needLength;
        } else {
            // うえ伸ばす
            rIdx.u += needLength;
        }
    } else if (dir == DIR::LEFT_DOWN) {
        // 左したのばす
        needLength = ctx->rng->next_uint32(1, 10);
        rIdx.l -= needLength;
        rIdx.d -= needLength;
    } else if (dir == DIR::RIGHT_UP) {
        // 右上のばす
        needLength = ctx->rng->next_uint32(1, 10);
        rIdx.r += needLength;
        rIdx.u += needLength;
    } else if (dir == DIR::LEFT_UP) {
        // 左上のばす
        needLength = ctx->rng->next_uint32(1, 10);
        rIdx.l -= needLength;
        rIdx.u += needLength;
    } else if (dir == DIR::RIGHT_DOWN) {
        // 右下のばす
        needLength = ctx->rng->next_uint32(1, 10);
        rIdx.r += needLength;
        rIdx.d -= needLength;
    }
    dir_dest = dir;
    pushLength = needLength;
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
    return Output(globalBest.rects);
}

void runMain(Args args, istream &is) {
    auto *timer = new RealTimer(TIME_LIMIT_SECONDS);
    auto *rng = new XorShift();
    auto *vis = new Visualizer(timer);
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
