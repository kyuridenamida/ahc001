#include <iostream>
#include <bitset>
#include <algorithm>
#include "./common.h"

using namespace std;


int random(int a, int b) {
    return rand() % (b - a + 1) + a;
}

class GreedySolver : Solver {
public:
    Output solve(Input input) {
//        reset();
        sort(input.advs.begin(), input.advs.end(), [&](const Adv &a, const Adv &b) {
            return a.r < b.r;
        });

        auto findBest = [&](Adv a, Size size) {
            for (int _ = 0; _ < 1000; _++) {
                int i = random(max(0, a.p.y - size.y + 1), a.p.y);
                int j = random(max(0, a.p.x - size.x + 1), a.p.x);
                if (i + size.y > input.H) {
                    continue;
                }
                if (j + size.x > input.W) {
                    continue;
                }
                auto r = Rect(P(j, i), size);
                if (!operations::cellCount(r)) {
                    return make_pair(true, Rect(P(j, i), size));
                }
            }
            return make_pair(false, Rect(P(-1, -1), size));
        };

        operations::resetAll();
        vector<OutputItem> res;
        for (auto a : input.advs) {
            auto size = Size::almostSquare(a.r);
            auto r = findBest(a, size);
            if (r.first) {
                res.emplace_back(a, r.second);
                operations::fill(r.second);
            }
        }
        return Output(input, res);
    }
};


int main() {
    auto a = loadFile("/home/kyuridenamida/ahc001/input/ex1.txt");
    const Input input = Input::fromInputStream(a);
    input.outputToStream(cerr);
    auto sol = GreedySolver().solve(input);
    sol.output(cout);
    cerr << sol.relativeScore() << endl;
}
