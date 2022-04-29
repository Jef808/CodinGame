#!/usr/bin/env python3

import sys
from pathlib import Path


optim_header = """
#undef _GLIBCXX_DEBUG // disable run-time bound checking, etc
#pragma GCC optimize("Ofast,inline") // Ofast = O3,fast-math,allow-store-data-races,no-protect-parens

#pragma GCC target("bmi,bmi2,lzcnt,popcnt") // bit manipulation
#pragma GCC target("movbe") // byte swap
#pragma GCC target("aes,pclmul,rdrnd") // encryption
#pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2") // SIMD
"""


def run(sources_fn, output_fn):

    with open(sources_fn) as sources:
        files = list(map(lambda src: source_dir / src.strip(), sources.readlines()))

    with open(output_fn, "w+") as out:
        out.write(optim_header)
        for file in files:
            with open(file) as f:
                for line in f.readlines():
                    if line.startswith('#include "'): continue
                    out.write(line)
                out.write('\n')


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(f"USAGE: {sys.argv[0]} SOURCES_DIRECTORY")
        exit(1)

    home_dir = Path.home()
    project_dir = home_dir / 'projects/CodinGame'
    source_dir = project_dir / sys.argv[1]
    sources = source_dir / 'sources.txt'
    output = source_dir / 'build/main_bundled.cpp'

    run(sources, output)
    exit(0)
