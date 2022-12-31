#!/usr/bin/env python3
"""
USAGE:

In a file `sources.txt`, list each source file needed to build your main
 (including any header files).
Note: If `sources.txt` is found in directory `DIR`, then all those files must also
 be there.

Executing this script with `DIR` as its only argument will produce a valid, single file called
main_bundled.cpp which can then be submitted on CodinGame.
"""

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


def run(src_dir, output_fn):
    """Read and concat all files found in src_dir/sources.txt"""
    with open(src_dir / "sources.txt", "r") as sources:
        files = list(map(lambda src: sources_dir / src.strip(),
                         sources.readlines()))

    with open(output_fn, "w+") as out:
        out.write(optim_header)
        for file in files:
            with open(file, encoding="utf8") as file:
                for line in file.readlines():
                    if line.startswith('#include "'):
                        continue
                    out.write(line)
                out.write('\n')


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(f"USAGE: {sys.argv[0]} SOURCES_DIRECTORY")
        sys.exit(1)

    sources_file = Path(sys.argv[1])
    sources_dir = sources_file.parent
    output = sources_dir / 'build/main_bundled.cpp'

    run(sources_dir, output)
    sys.exit(0)
