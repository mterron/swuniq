#!/usr/bin/env python3

# Suite borrowed from https://github.com/cjm00/dedup

# This file must live in swuniq/benchsuite! The file may be run from anywhere, but the script itself must reside in this subdirectory.

import os
import subprocess
import gzip
import argparse
import time
import platform
import shutil
# import tempfile
import tarfile
import hashlib

from collections import defaultdict
# from operator import itemgetter
from statistics import mean, pstdev
from zipfile import ZipFile

BENCH_FILE_PATH = os.path.abspath(__file__)
BENCH_DIR = os.path.dirname(BENCH_FILE_PATH)
PROJECT_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), os.path.pardir))

TEST_DIR =  "corpustest"

EUROPARL_BLOB = "europarl.txt"
EUROPARL_ARCHIVE = "en.raw.tar.gz"
EUROPARL_URL = "opus.nlpl.eu/download.php?f=Europarl/en.raw.tar.gz"

ENWIKI9_BLOB = "enwik9"
ENWIKI9_ARCHIVE = "enwik9.zip"
ENWIKI9_URL = "http://mattmahoney.net/dc/enwik9.zip"

SWUNIQ_BIN_NAME = "swuniq" + (".exe" if platform.system() == 'Windows' else "")
SWUNIQ_BIN_PATH = os.path.join(PROJECT_PATH, "bin", SWUNIQ_BIN_NAME)

SORT_PATH = "sort"

class MissingCommands(Exception):
    def __init__(self, missing_names):
        self.missing = missing_names

    def __str__(self):
        return "MissingCommands({})".format(repr(self.missing))

class Command():
    def __init__(self, name, cmd, *args, **kwargs):
        self.name = name
        self.cmd = cmd
        self.args = args
        self.kwargs = kwargs

    def exists(self):
        return shutil.which(self.binary_name) is not None

    @property
    def binary_name(self):
        return self.cmd[0]

    def run(self):
        return subprocess.run(self.cmd, *self.args, **self.kwargs)


class Benchmarker():
    def __init__(self, warmup=1, count=3, commands=None, directory=None, use_tempfile=False, count_lines=False):
        self.warmup = warmup
        self.count = count
        self.commands = commands or []
        self.directory = directory
        self.use_tempfile = use_tempfile
        self.count_lines = count_lines

    def check_if_missing(self):
        missing = []
        for cm in self.commands:
            if not cm.exists():
                missing.append(cm.binary_name)

        if missing:
            raise MissingCommands(missing)

    def run_once(self, cmd):
        if not cmd.exists():
            raise MissingCommands([cmd.binary_name])

        cmd.kwargs["stdout"] = subprocess.DEVNULL
        cmd.kwargs["cwd"] = self.directory

        start = time.time()
        res = cmd.run()
        end = time.time()


        line_count = None

        if self.count_lines:
            line_count = res.stdout.count(b'\n')

        return {
            'duration' : end - start,
            'line_count' : line_count
        }

    def run(self):
        self.check_if_missing()

        times = defaultdict(list)

        for cmd in self.commands:
            for _ in range(self.warmup):
                self.run_once(cmd)
            for _ in range(self.count):
                times[cmd.name].append(self.run_once(cmd)["duration"])

        return times

def bench_europarl_to_stdout(config, args):

    BLOB_PATH = os.path.join(config["directory"], EUROPARL_BLOB)

    cmds = []

    if args.run in ["sort", "all"]:
        cmds.append(
            Command("sort", [SORT_PATH, "-u", BLOB_PATH])
        )
    if args.run in ["awk", "all"]:
        cmds.append(
            Command('awk', ['awk', '!seen[$0]++', BLOB_PATH])
        )
    if args.run in ["swuniq", "all"]:
        cmds.append(
            Command("swuniq", ['swuniq', BLOB_PATH])
        )
        cmds.append(
            Command("swuniq -w 20", ['swuniq', '-w', '20', BLOB_PATH])
        )
        cmds.append(
            Command("swuniq -w 10", ['swuniq', '-w', '10', BLOB_PATH])
        )
        cmds.append(
            Command("swuniq -w 1", ['swuniq', '-w', '1', BLOB_PATH])
        )

    bencher = Benchmarker(commands=cmds, **config)
    benches = bencher.run()

    for k, v in benches.items():
        print("{:8} - Fastest Run: {:<04.3}s  Average Run Time: {:<04.3}s  Stddev: {:<04.3}".format(k, min(v), mean(v), pstdev(v)))

def bench_enwiki9_to_stdout(config, args):

    BLOB_PATH = os.path.join(config["directory"], ENWIKI9_BLOB)

    cmds = []

    if args.run in ["sort", "all"]:
        cmds.append(
            Command("sort", [SORT_PATH, "-u", BLOB_PATH])
        )
    if args.run in ["awk", "all"]:
        cmds.append(
            Command('awk', ['awk', '!seen[$0]++', BLOB_PATH])
        )
    if args.run in ["swuniq", "all"]:
        cmds.append(
            Command("swuniq", ['swuniq', BLOB_PATH])
        )
        cmds.append(
            Command("swuniq -w 20", ['swuniq', '-w', '20', BLOB_PATH])
        )
        cmds.append(
            Command("swuniq -w 10", ['swuniq', '-w', '10', BLOB_PATH])
        )
        cmds.append(
            Command("swuniq -w 1", ['swuniq', '-w', '1', BLOB_PATH])
        )

    bencher = Benchmarker(commands=cmds, **config)
    benches = bencher.run()

    for k, v in benches.items():
        print("{:8} - Fastest Run: {:<04.3}s  Average Run Time: {:<04.3}s  Stddev: {:<04.3}".format(k, min(v), mean(v), pstdev(v)))


def download_corpus(directory, url, archive_name):
    corpus_archive = os.path.join(directory, archive_name)

    if not os.path.isdir(directory):
        os.makedirs(directory)
    if not os.path.exists(corpus_archive):
        if not os.path.exists(archive_name):
            print("Downloading test corpus...")
            subprocess.run(['curl', url, '-L', '--output', corpus_archive], cwd=directory)

def unpack_verify_enwiki9(testdir, archive_name, blob_name):
    archive_path = os.path.join(testdir, archive_name)
    raw_path = os.path.join(testdir, "enwik9")
    blob_path = os.path.join(testdir, blob_name)
    print("Verifying enwiki9 corpus...")
    if not verify_enwiki9_corpus(blob_path):
        print("Extracting enwiki9 corpus...")
        with ZipFile(archive_path) as enwiki_zip:
            enwiki_zip.extract("enwik9", path=testdir)
        os.rename(raw_path, blob_path)

def unpack_verify_europarl(testdir, archive_name, blob_name):
    archive_path = os.path.join(testdir, archive_name)
    blob_path = os.path.join(testdir, blob_name)

    print("Verifying europarl corpus...")
    if not verify_europarl_corpus(blob_path):
        pack_tarfile_into_txt(archive_path, blob_path)

def pack_tarfile_into_txt(tarfile_path, txt_path):
    print("Building corpus from archive...")
    with open(txt_path, "w+b") as txt:
        with tarfile.open(tarfile_path, mode="r:gz") as archive:
            for member_name in archive.getnames():
                member_file = archive.extractfile(member_name)
                if member_file is None: continue
                with gzip.open(member_file, "rb") as part:
                    txt.write(part.read())

def verify_enwiki9_corpus(corpus_location):
    corpushash = '159b85351e5f76e60cbe32e04c677847a9ecba3adc79addab6f4c6c7aa3744bc'
    if not os.path.exists(corpus_location):
        return False

    return hash_file(corpus_location) == corpushash

def verify_europarl_corpus(corpus_location):
    corpushash = '74a318fd3fb86d0297661ce3516126d61db19a6bf1692893b71025d11e4116e5'
    if not os.path.exists(corpus_location):
        return False

    return hash_file(corpus_location) == corpushash


def hash_file(file_path):
    with open(file_path, "rb") as corpus:
        h = hashlib.sha256()
        for block in iter(lambda: corpus.read(8 * 1000 * 1000), b""):
            h.update(block)
    return h.hexdigest()

def build_swuniq():
    print("Building swuniq...")
    proj_path = os.path.abspath(os.path.join(os.path.dirname(__file__), os.path.pardir))
    subprocess.run(['gcc', '-O', 'swuniq.c', '-o', 'bin/swuniq'], cwd=proj_path)


def main():
    p = argparse.ArgumentParser("Command line deduplication tool benchmark runner.")
    p.add_argument(
        '--warmup', metavar='INTEGER', type=int, default=1,
        help='The number of iterations to run each command before '
        'the actual measurements begin.')
    p.add_argument(
        '--benches', metavar='INTEGER', type=int, default=3,
        help='The number of samples to take when performing measurements.'
    )
    p.add_argument(
        '--sortpath', metavar='SORTPATH', default='sort'
    )
    p.add_argument(
        '--run', metavar='TESTNAME', choices=["awk", "sort", "swuniq", "all"], nargs='?', default="all"
    )
    p.add_argument(
        '--dir', metavar='TESTDIR', default=BENCH_DIR
    )
    p.add_argument(
        '--corpus', choices=["europarl", "wiki", "all"], nargs='?', default="europarl"
    )
    args = p.parse_args()

    global SORT_PATH
    SORT_PATH = args.sortpath

    config = dict()
    config["count"] = args.benches
    config["warmup"] = args.warmup
    config["directory"] = os.path.join(args.dir, TEST_DIR)

    if args.corpus in ("europarl", "all"):
        download_corpus(config["directory"], EUROPARL_URL, EUROPARL_ARCHIVE)
        unpack_verify_europarl(config["directory"], EUROPARL_ARCHIVE, EUROPARL_BLOB)
        bench_europarl_to_stdout(config, args)

    if args.corpus in ("wiki", "all"):
        download_corpus(config["directory"], ENWIKI9_URL, ENWIKI9_ARCHIVE)
        unpack_verify_enwiki9(config["directory"], ENWIKI9_ARCHIVE, ENWIKI9_BLOB)
        bench_enwiki9_to_stdout(config, args)

if __name__ == '__main__':
    main()
