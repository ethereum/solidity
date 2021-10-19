#!/usr/bin/env python3
#
# - SolidityEndToEndTest.trace was created with soltest with the following command on
#     ./soltest --color_output=false --log_level=test_suite -t SolidityEndToEndTest/ -- --no-smt
#         --evmonepath /Users/alex/evmone/lib/libevmone.dylib --show-messages > SolidityEndToEndTest.trace
# - a trace of the semantic tests can be created by using
#     ./soltest --color_output=false --log_level=test_suite -t semanticTests/extracted/ -- --no-smt
#         --evmonepath /Users/alex/evmone/lib/libevmone.dylib --show-messages > semanticTests.trace
#
# verify-testcases.py will compare both traces. If these traces are identical, the extracted tests were
# identical with the tests specified in SolidityEndToEndTest.cpp.
#
# pylint: disable=too-many-instance-attributes

import re
import os
import sys
import getopt
import json


class Trace:
    def __init__(self, kind, parameter):
        self.kind = kind
        self.parameter = parameter
        self._input = ""
        self._output = ""
        self.value = ""
        self.result = ""
        self.gas = ""

    def get_input(self):
        return self._input

    def set_input(self, input):
        if self.kind == "create":
            # remove cbor encoded metadata from bytecode
            length = int(input[-4:], 16) * 2
            self._input = input[:len(input) - length - 4]

    def get_output(self):
        return self._output

    def set_output(self, output):
        if self.kind == "create":
            # remove cbor encoded metadata from bytecode
            length = int(output[-4:], 16) * 2
            self._output = output[:len(output) - length - 4]

    def __str__(self):
        # we ignore the used gas
        result = str(
            "kind='" + self.kind + "' parameter='" + self.parameter + "' input='" + self._input +
            "' output='" + self._output + "' value='" + self.value + "' result='" + self.result + "'"
        )
        return result


class TestCase:
    def __init__(self, name):
        self.name = name
        self.metadata = None
        self.traces = []

    def add_trace(self, kind, parameter):
        trace = Trace(kind, parameter)
        self.traces.append(trace)
        return trace


class TraceAnalyser:
    def __init__(self, file):
        self.file = file
        self.tests = {}
        self.ready = False

    def analyse(self):
        with open(self.file, "r", encoding='utf8') as trace_file:
            trace = None
            test_case = None
            for line in trace_file.readlines():
                test = re.search(r'Entering test case "(.*)"', line, re.M | re.I)
                if test:
                    test_name = test.group(1)
                    test_case = TestCase(test_name)
                    self.tests[test_name] = test_case

                metadata = re.search(r'\s*metadata:\s*(.*)$', line, re.M | re.I)
                if metadata:
                    test_case.metadata = json.loads(metadata.group(1))
                    del test_case.metadata["sources"]
                    del test_case.metadata["compiler"]["version"]

                create = re.search(r'CREATE\s*([a-fA-F0-9]*):', line, re.M | re.I)
                if create:
                    trace = test_case.add_trace("create", create.group(1))

                call = re.search(r'CALL\s*([a-fA-F0-9]*)\s*->\s*([a-fA-F0-9]*):', line, re.M | re.I)
                if call:
                    trace = test_case.add_trace("call", call.group(1))  # + "->" + call.group(2))

                if not create and not call:
                    self.parse_parameters(line, trace)

            trace_file.close()

            print(self.file + ":", len(self.tests), "test-cases.")

            self.ready = True

    @staticmethod
    def parse_parameters(line, trace):
        input = re.search(r'\s*in:\s*([a-fA-F0-9]*)', line, re.M | re.I)
        if input:
            trace.input = input.group(1)
        output = re.search(r'\s*out:\s*([a-fA-F0-9]*)', line, re.M | re.I)
        if output:
            trace.output = output.group(1)
        result = re.search(r'\s*result:\s*([a-fA-F0-9]*)', line, re.M | re.I)
        if result:
            trace.result = result.group(1)
        gas_used = re.search(r'\s*gas\sused:\s*([a-fA-F0-9]*)', line, re.M | re.I)
        if gas_used:
            trace.gas = gas_used.group(1)
        value = re.search(r'\s*value:\s*([a-fA-F0-9]*)', line, re.M | re.I)
        if value:
            trace.value = value.group(1)

    def diff(self, analyser):
        if not self.ready:
            self.analyse()
        if not analyser.ready:
            analyser.analyse()

        intersection = set(self.tests.keys()) & set(analyser.tests.keys())
        mismatches = set()

        for test_name in intersection:
            left = self.tests[test_name]
            right = analyser.tests[test_name]
            if json.dumps(left.metadata) != json.dumps(right.metadata):
                mismatches.add(
                    (test_name, "metadata where different: " + json.dumps(left.metadata) + " != " + json.dumps(
                        right.metadata)))
            if len(left.traces) != len(right.traces):
                mismatches.add((test_name, "trace count are different: " + str(len(left.traces)) +
                                " != " + str(len(right.traces))))
            else:
                self.check_traces(test_name, left, right, mismatches)

        for mismatch in mismatches:
            print(mismatch[0])
            print(mismatch[1])

        print(len(intersection), "test-cases - ", len(mismatches), " mismatche(s)")

    def check_traces(self, test_name, left, right, mismatches):
        for trace_id, trace in enumerate(left.traces):
            left_trace = trace
            right_trace = right.traces[trace_id]
            assert left_trace.kind == right_trace.kind
            if str(left_trace) != str(right_trace):
                mismatch_info = "    " + str(left_trace) + "\n"
                mismatch_info += "    " + str(right_trace) + "\n"
                mismatch_info += "    "
                for ch in range(0, len(str(left_trace))):
                    if ch < len(str(left_trace)) and ch < len(str(right_trace)):
                        if str(left_trace)[ch] != str(right_trace)[ch]:
                            mismatch_info += "|"
                        else:
                            mismatch_info += " "
                    else:
                        mismatch_info += "|"
                mismatch_info += "\n"
                mismatches.add((test_name, mismatch_info))


def main(argv):
    extracted_tests_trace_file = None
    end_to_end_trace_file = None
    try:
        opts, _args = getopt.getopt(argv, "s:e:")
    except getopt.GetoptError:
        print("verify-testcases.py [-s <path to semantic-trace>] [-e <path to endToEndExtraction-trace>]")
        sys.exit(2)

    for opt, arg in opts:
        if opt in '-s':
            extracted_tests_trace_file = arg
        elif opt in '-e':
            end_to_end_trace_file = arg

    base_path = os.path.dirname(__file__)
    if not extracted_tests_trace_file:
        extracted_tests_trace_file = base_path + "/extracted-tests.trace"
    if not end_to_end_trace_file:
        end_to_end_trace_file = base_path + "/endToEndExtraction-tests.trace"

    for f in [extracted_tests_trace_file, end_to_end_trace_file]:
        if not os.path.isfile(f):
            print("trace file '" + f + "' not found. aborting.")
            sys.exit(1)

    if not os.path.isfile(extracted_tests_trace_file):
        print("semantic trace file '" + extracted_tests_trace_file + "' not found. aborting.")
        sys.exit(1)

    semantic_trace = TraceAnalyser(extracted_tests_trace_file)
    end_to_end_trace = TraceAnalyser(end_to_end_trace_file)

    semantic_trace.diff(end_to_end_trace)


if __name__ == "__main__":
    main(sys.argv[1:])
