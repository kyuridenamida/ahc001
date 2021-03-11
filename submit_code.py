import os
import sys


def removeDebugCode(src):
    lines = src.split("\n")
    res = []
    for l in lines:
        if "emitJson(" in l or '#include "' in l or ("emitJsonWithTimer(" in l and "void" not in l) or "cerr" in l:
            print(f"Warn: The following Line  will be remove! [{l}]", file=sys.stderr)
        else:
            res += [l]
    return "\n".join(res)


def replaceInclude(src, includeName):
    with open(os.path.join("src", includeName)) as f:
        return src.replace(f'#include "{includeName}"', f.read())

if __name__ == "__main__":
    with open("src/main.cpp", "r") as f:
        src = f.read()

        src = replaceInclude(src, "./common.h")
        src = replaceInclude(src, "./Timer.h")
        src = removeDebugCode(src)

        print(src)