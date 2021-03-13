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

inline bool overlap(int a, int b, int A, int B) {
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

    int manhattanDist(const P &op){
        return abs(op.x - x) + abs(op.y - y);
    }
};

struct Adv {
    int id;
    P p;
    int r;
};

class Rect {
public:
    short l;
    short r;
    short d;
    short u;

    Rect() {}

    Rect(short l, short r, short d, short u) : l(l), r(r), d(d), u(u) {}

    int area() {
        return (int) (r - l) * (u - d);
    }

    static Rect onePixelRect(const P &leftDownP) {
        return Rect(leftDownP.x, leftDownP.x + 1, leftDownP.y, leftDownP.y + 1);
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

#endif //AHC001_SOLUTION_H
