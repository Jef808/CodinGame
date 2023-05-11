#!/usr/bin/env python3
"""See __file__ --help."""

import argparse
from math import nan as NaN
import re
from pathlib import Path
from os import access, R_OK, W_OK, X_OK

offline_header = """#define _OFFLINE  // Flag indicating offline usage
                  // (e.g. for viewer)
"""

optim_header = """#undef _GLIBCXX_DEBUG  // disable run-time bound checking,etc
#pragma GCC optimize("Ofast,inline")  // Ofast = O3,fast-math,
                                      // allow-store-data-races,
                                      // no-protect-parens
# pragma GCC target("bmi,bmi2,lzcnt,popcnt")  // bit manipulation
# pragma GCC target("movbe")  // byte swap
# pragma GCC target("aes,pclmul,rdrnd")  // encryption
# pragma GCC target("avx,avx2,f16c,fma,sse3,ssse3,sse4.1,sse4.2")  // SIMD
"""


class PathType(object):
    """PathType."""

    def __init__(self, exists=True, type='file', permissions=[]):
        """Init."""
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
        """Call."""
        self._validate_existence(string)
        self._validate_type(string)
        self._validate_permissions(string)
        return string


class SourcesDir(PathType):
    """SourcesDir."""

    def __init__(self):
        """Init."""
        super().__init__(type='dir', exists=True)

    def __call__(self, string: str):
        """Call."""
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
    """Read and concat all files found in src_dir/sources.txt."""
    dir = sources.parent
    with open(sources, "r") as sources:
        files = list(map(lambda source: dir / source.strip(),
                         sources.readlines()))
    include_directives = []
    offline_include_directives = []
    include_directive_pattern = re.compile(
        r'^#include ([<"])[\w./]+([>"])$'
    )
    result = []
    current_preprocessor_directive_if_depth = 0
    running_offline_preprocessor_condition_depth = NaN
    header_guard_depth = NaN
    is_next_after_header_guard_start = False
    for file in files:
        with open(file, encoding="utf8") as file:
            for line in (_line.rstrip() for _line in file.readlines()):
                if not line:
                    continue

                if is_next_after_header_guard_start and line.startswith('#define'):
                    is_next_after_header_guard_start = False
                    continue

                is_ignored_line = False

                if line.startswith('#endif'):
                    is_header_guard_end = (
                        current_preprocessor_directive_if_depth
                        == header_guard_depth)
                    is_running_offline_preprocessor_condition_end = (
                        current_preprocessor_directive_if_depth
                        == running_offline_preprocessor_condition_depth)

                    current_preprocessor_directive_if_depth -= 1

                    if is_header_guard_end:
                        header_guard_depth = NaN
                        is_ignored_line = True

                    if is_running_offline_preprocessor_condition_end:
                        running_offline_preprocessor_condition_depth = NaN
                        is_ignored_line = True

                elif line.startswith('#if'):
                    current_preprocessor_directive_if_depth += 1

                    is_header_guard_start = (
                        line[3:].startswith('ndef')
                        and line.count('_H' or '_HPP'))
                    is_running_offline_preprocessor_condition_start = (
                        line[3:].startswith(' RUNNING_OFFLINE'))

                    if is_header_guard_start:
                        header_guard_depth = (
                            current_preprocessor_directive_if_depth
                        )
                        is_next_after_header_guard_start = True
                        is_ignored_line = True

                    elif is_running_offline_preprocessor_condition_start:
                        running_offline_preprocessor_condition_depth = (
                            current_preprocessor_directive_if_depth
                        )
                        is_ignored_line = True

                if not is_ignored_line:
                    include_directive_match = (
                        re.match(include_directive_pattern, line))
                    is_maybe_include_directive = (
                        include_directive_match is not None
                    )
                    is_local_include_directive = (
                        is_maybe_include_directive and
                        include_directive_match.group(1) == '"' and
                        include_directive_match.group(2) == '"'
                    )
                    is_system_include_directive = (
                        is_maybe_include_directive and
                        include_directive_match.group(1) == '<' and
                        include_directive_match.group(2) == '>'
                    )
                    is_include_directive = (is_local_include_directive
                                            or is_system_include_directive)
                    if is_include_directive:
                        is_in_offline_preprocessor_condition_block = (
                            current_preprocessor_directive_if_depth
                            >= running_offline_preprocessor_condition_depth
                        )
                        if is_system_include_directive:
                            include_directives_array = (
                                offline_include_directives
                                if is_in_offline_preprocessor_condition_block
                                else include_directives
                            )
                            include_directives_array.append(line)
                    else:
                        result.append(line)

    with open(output_filepath, "w+") as out:
        out.write(optim_header)
        if offline:
            out.write(offline_header)
        out.write('\n')
        out.write('\n'.join(sorted(set(include_directives))))
        out.write('\n#if RUNNING_OFFLINE\n')
        out.write('\n'.join(sorted(set(offline_include_directives))))
        out.write('\n#endif\n')
        out.write('\n'.join(result))


if __name__ == '__main__':
    args = parser.parse_args()
    sources_dir = (Path.cwd() if args.sources_dir is None
                   else Path(args.sources_dir).absolute())
    sources = sources_dir / "sources.txt"
    output = Path(args.output_directory).absolute() / args.output_filename
    if not output.suffix:
        output = output.with_suffix(".cpp")

    main(sources, output, args.offline)
