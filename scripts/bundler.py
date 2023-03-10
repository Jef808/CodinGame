#!/usr/bin/env python3
"""
USAGE:

See __file__ --help.
"""

import argparse
from pathlib import Path
from os import access, R_OK, W_OK, X_OK

offline_header = "#define _OFFLINE\n\n"

optim_header = """
#undef _GLIBCXX_DEBUG // disable run-time bound checking, etc
#pragma GCC optimize("Ofast,inline") // Ofast = O3,fast-math,allow-store-data-races,no-protect-parens

#pragma GCC target("bmi,bmi2,lzcnt,popcnt") // bit manipulation
#pragma GCC target("movbe") // byte swap
#pragma GCC target("aes,pclmul,rdrnd") // encryption
#pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2") // SIMD
"""


class PathType(object):
    def __init__(self, exists=True, type='file', permissions=[]):
        assert exists in (True, False, None)
        assert type in ('file', 'dir', None) or hasattr(type, '__call__')
        assert all(perm in ('R_OK', 'W_OK', 'X_OK') for perm in permissions)

        self._exists = exists
        self._type = type
        self._permissions = permissions

    def _validate_type(self, string: str):
        prospective_path = Path(string)
        if self._type is None:
            return
        elif self._type == 'file':
            if not prospective_path.is_file():
                raise argparse.ArgumentError(
                    f"path_type: {prospective_path} is not a valid file")
        elif self._type == 'dir':
            if not prospective_path.is_dir():
                raise argparse.ArgumentError(
                    f"path_type: {prospective_path} is not a valid directory")

    def _validate_permissions(self, string: str):
        prospective_path = Path(string)
        if 'R_OK' in self._permissions:
            if not access(prospective_path, R_OK):
                raise argparse.ArgumentError(
                    f"writable_dir: {prospective_path} is not readable")
        if 'W_OK' in self._permissions:
            if not access(prospective_path, W_OK):
                raise argparse.ArgumentError(
                    f"writable_dir: {prospective_path} is not writable")
        if 'X_OK' in self._permissions:
            if not access(prospective_path, X_OK):
                raise argparse.ArgumentError(
                    f"writable_dir: {prospective_path} is not executable")

    def _validate_existence(self, string: str):
        prospective_path = Path(string)
        if self._exists is not None:
            if self._exists:
                if not prospective_path.exists():
                    raise argparse.ArgumentError(
                        f"path_type: {prospective_path} does not exists")
            elif prospective_path.exists():
                raise argparse.ArgumentError(
                    f"path_type: {prospective_path} already exists")

    def __call__(self, string: str):
        self._validate_existence(string)
        self._validate_type(string)
        self._validate_permissions(string)
        return string


class SourcesDir(PathType):
    def __init__(self):
        super().__init__(type='dir', exists=True)

    def __call__(self, string: str):
        super().__call__(string)
        sources_dir = Path(string)
        sources_file = sources_dir / "sources.txt"
        if not (sources_dir.is_dir()):
            raise argparse.ArgumentError(
                f"sources_dir: {sources_dir} is not a valid directory")
        if not (sources_file.is_file() or not access(sources_file, R_OK)):
            raise argparse.ArgumentError(
                f"sources_dir: {sources_file} is not a valid readable file")
        return string


parser = argparse.ArgumentParser(
                    description="Bundles a collection of headers and sources "
                                "into one .cpp file")

parser.add_argument(
    '-d',
    help='output directory',
    action='store',
    type=PathType(exists=True,
                  type='dir',
                  permissions=['W_OK']),
    default='./build/bin',
    dest='output_directory')

parser.add_argument(
    '-o',
    help='output filename',
    action='store',
    default='main_bundled',
    dest='output_filename')

parser.add_argument(
    '-s',
    help='directory containing the sources.txt file',
    action='store',
    type=SourcesDir(),
    default=None,
    dest='sources_dir')

parser.add_argument(
    '--offline',
    help='flag indicating that the output is for offline use',
    action='store_true')


def main(sources: Path, output_filepath: Path, offline: bool):
    """Read and concat all files found in src_dir/sources.txt"""
    dir = sources.parent
    with open(sources, "r") as sources:
        files = list(map(lambda source: dir / source.strip(),
                         sources.readlines()))
    with open(output_filepath, "w+") as out:
        out.write(optim_header)
        if offline:
            out.write(offline_header)
        for file in files:
            with open(file, encoding="utf8") as file:
                for line in file.readlines():
                    if line.startswith('#include "'):
                        continue
                    out.write(line)
                out.write('\n')


if __name__ == '__main__':
    args = parser.parse_args()
    sources_dir = (Path.cwd() if args.sources_dir is None
                   else Path(args.sources_dir))
    sources = sources_dir / "sources.txt"
    output = Path(args.output_directory) / args.output_filename
    if not output.suffix:
        output = output.with_suffix(".cpp")
    main(sources, output, args.offline)
