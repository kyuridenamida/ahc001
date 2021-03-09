#include <iostream>
#include <bitset>
#include <algorithm>
#include <complex>
#include "./common.h"

using namespace std;


typedef complex<double> GeoPoint;
typedef GeoPoint V;

int random(int a, int b) {
    return rand() % (b - a + 1) + a;
}

double f(double a, double b, double A, double B) {
    if (min(b, B) - max(a, A) <= 0) {
        return 0;
    }

    // TODO いろいろ試す
    return a + b < A + B ? -1 : +1;
}

bool eq(double a, double b) {
    return abs(a - b) < 0.0001;
}

class GeoRect {

public:
    double l;
    double r;
    double d;
    double u;

    GeoRect() { l = r = d = u - 1; }

    GeoRect(double l, double r, double d, double u) : l(l), r(r), d(d), u(u) {}

    double area() {
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
        ans += min(rects[i].area(), 1. * input.advs[i].r) / max(rects[i].area(), 1. * input.advs[i].r);
    }
    return ans / rects.size();
}

vector<GeoRect> next(vector<GeoRect> rects, const Input &input) {
    vector<double> VX(rects.size());
    vector<double> VY(rects.size());

    for (int i = 0; i < rects.size(); i++) {
        for (int j = 0; j < rects.size(); j++) {
            if (i != j) {
                double X = f(rects[i].l, rects[i].r, rects[j].l, rects[j].r);
                double Y = f(rects[i].d, rects[i].u, rects[j].d, rects[j].u);
                if (X != 0 && Y != 0) {
                    VX[i] += X;
                    VY[i] += Y;
                }
            }
        }
    }

    for (int i = 0; i < rects.size(); i++) {
        rects[i].l += VX[i];
        rects[i].r += VX[i];
        rects[i].d += VY[i];
        rects[i].u += VY[i];
    }

    for (int i = 0; i < rects.size(); i++) {
        rects[i].l = max(0.0, rects[i].l);
        rects[i].d = max(0.0, rects[i].d);
        rects[i].r = min(10000.0, rects[i].r);
        rects[i].u = min(10000.0, rects[i].u);

        auto q = input.advs[i].p;

        rects[i].l = min<double>(rects[i].l, q.x);
        rects[i].r = max<double>(rects[i].r, q.x);
        rects[i].d = min<double>(rects[i].d, q.y);
        rects[i].u = max<double>(rects[i].u, q.y);
    }

    for (int i = 0; i < rects.size(); i++) {
        if (rects[i].l > rects[i].r)
            swap(rects[i].l, rects[i].r);
        if (rects[i].d > rects[i].u)
            swap(rects[i].u, rects[i].d);
    }
    return rects;
}

double simulation() {

}


Output createOutput(vector<GeoRect> rects, Input input) {
    vector<OutputItem> res;
    for (auto i : input.advs) {
        res.emplace_back(i, rects[i.id].toRect());
    }
    return Output(input, res);
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
        while (true) {
            vector<GeoRect> r = rects;
            for (int t = 0; t < 10; t++) {
                int idx = rand() % input.n;

                double A =
                        min(rects[idx].area(), 1. * input.advs[idx].r) / max(rects[idx].area(), 1. * input.advs[idx].r);
                if (A > 0.9) continue;

                r[idx].l -= random(0, 10);
                r[idx].r += random(0, 10);
                r[idx].d -= random(0, 10);
                r[idx].u += random(0, 10);
            }

            for (int j = 0; j < 1000; j++) {
                vector<GeoRect> nxt = next(r, input);
                r = nxt;
            }
            double nextScore = score(r, input);

            if (nextScore > currentScore - 0.001) {
                cerr << currentScore << endl;
                currentScore = nextScore;
                rects = r;
                if (currentScore > 0.66) {
                    createOutput(rects, input).output(cout);
                    break;
                }
            }

        }
    }
};


int main() {
    auto a = loadFile("/home/kyuridenamida/ahc001/input/ex1.txt");
    const Input input = Input::fromInputStream(a);
    input.outputToStream(cerr);
    auto sol = PhysicsSolver().solve(input);
    sol.output(cout);
    cerr << sol.relativeScore() << endl;
}
