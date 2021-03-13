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
};

struct Adv {
    int id;
    P p;
    int r;
};

class Rect {
public:
    int l;
    int r;
    int d;
    int u;

    Rect() {}

    Rect(int l, int r, int d, int u) : l(l), r(r), d(d), u(u) {}

    int area() {
        return (r - l) * (u - d);
    }

    static Rect onePixelRect(const P &leftDownP) {
        return {leftDownP.x, leftDownP.x + 1, leftDownP.y, leftDownP.y + 1};
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

    /**
     * @return if the update is valid
     */
    int cnt = 0;
    int ok = 0;

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
                        cerr << (geoRect != geoRect_) << " " << pushLength << " " << pushDir << endl;
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

#endif //AHC001_SOLUTION_H
