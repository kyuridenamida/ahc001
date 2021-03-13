#!/usr/bin/env python3
import itertools
import sys

if __name__ == "__main__":
    args = sys.argv[1:]
    lists = [x.split(",") for x in args]
    for x in itertools.product(*lists):
        print(" ".join(x))
