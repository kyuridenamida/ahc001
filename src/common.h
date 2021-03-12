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

    void output(ostream &os) {
        // TODO: 座標のinclusive / exclusive
        auto cpy = items;
        vector<Rect> answer(input.n);


        for (auto item : items) {
            answer[item.adv.id] = item.r;
        }

        vector<Rect> rs;

        for (auto r : answer) {
            os << r.p.x << " " << r.p.y << " " << r.p.x + r.size.x << " "
               << r.p.y + r.size.y << endl;
        }
    }
};


class Solver {
public:
    virtual Output solve(Input request) = 0;
};

#endif //AHC001_SOLUTION_H
