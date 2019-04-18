#!/usr/bin/env python3

from argparse import ArgumentParser
import sys
import shutil
import os
import subprocess
import re
import glob

DESCRIPTION = """Regressor is a tool to run regression tests in a CI env."""


class regressor():
    _re_sanitizer_log = re.compile(r"""ERROR: (?P<sanitizer>\w+).*""")

    def __init__(self, description, args):
        self._description = description
        self._args = self.parseCmdLine(description, args)
        self._repo_root = os.path.dirname(sys.path[0])
        self._fuzzer_path = os.path.join(self._repo_root,
                                         "build/test/tools/ossfuzz")
        self._logpath = os.path.join(self._repo_root, "test_results")

    def parseCmdLine(self, description, args):
        argParser = ArgumentParser(description)
        argParser.add_argument('-o', '--out-dir', required=True, type=str,
                       help="""Directory where test results will be written""")
        return argParser.parse_args(args)

    @staticmethod
    def run_cmd(command, logfile=None, env=None):
        if not logfile:
            logfile = os.devnull

        if not env:
            env = os.environ.copy()

        logfh = open(logfile, 'w')
        proc = subprocess.Popen(command, shell=True, executable='/bin/bash',
                                env=env, stdout=logfh,
                                stderr=subprocess.STDOUT)
        ret = proc.wait()
        logfh.close()

        if ret != 0:
            return False
        return True

    def process_log(self, logfile):
        list = re.findall(self._re_sanitizer_log, open(logfile, 'r').read())
        numSuppressedLeaks = list.count("LeakSanitizer")
        return "AddressSanitizer" not in list, numSuppressedLeaks

    def run(self):
        for fuzzer in glob.iglob("{}/*_ossfuzz".format(self._fuzzer_path)):
            basename = os.path.basename(fuzzer)
            logfile = os.path.join(self._logpath, "{}.log".format(basename))
            corpus_dir = "/tmp/solidity-fuzzing-corpus/{0}_seed_corpus" \
                .format(basename)
            cmd = "find {0} -type f | xargs {1}".format(corpus_dir, fuzzer)
            if not self.run_cmd(cmd, logfile=logfile):
                ret, numLeaks = self.process_log(logfile)
                if not ret:
                    print(
                        "\t[-] AddressSanitizer reported failure for {0}. "
                        "Failure logged to test_results".format(
                            basename))
                    return False
                else:
                    print("\t[+] {0} passed regression tests but leaked "
                          "memory.".format(basename))
                    print("\t\t[+] Suppressed {0} memory leak reports".format(
                            numLeaks))
            else:
                print("\t[+] {0} passed regression tests.".format(basename))
        return True


if __name__ == '__main__':
    tool = regressor(DESCRIPTION, sys.argv[1:])
    tool.run()
