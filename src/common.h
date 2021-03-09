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
