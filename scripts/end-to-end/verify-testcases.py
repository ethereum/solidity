#!/usr/bin/env python3
#
# - SolidityEndToEndTest.trace was created with soltest with the following command on
# develop @ commit 9318dae42cfd1a0f048665ec1eafae730bf7cb6b:
#     ./soltest --log_level=test_suite -t SolidityEndToEndTest/ -- --no-smt
#         --evmonepath /Users/alex/evmone/lib/libevmone.dylib --show-messages > SolidityEndToEndTest.trace
# - a trace of the semantic tests can be created by using
#     ./soltest --log_level=test_suite -t semanticTests/ -- --no-smt
#         --evmonepath /Users/alex/evmone/lib/libevmone.dylib --show-messages > semanticTests.trace
#
# verify-testcases.py will compare both traces. If these traces are identical, the extracted tests where
# identical with the tests specified in SolidityEndToEndTest.cpp.
#
import re
import os


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
        result = str(
            # "kind='" + self.kind + "' parameter='" + self.parameter + "' input='" + self.input + "' output='" + self.output + "' value='" + self.value + "' result='" + self.result + "' gas='" + self.gas + "'"
            "kind='" + self.kind + "' parameter='" + self.parameter + "' input='" + self._input + "' output='" + self._output + "' value='" + self.value + "' result='" + self.result + "'"
        )
        # result = self.kind+":"+str(hash(result))
        return result


class TestCase:
    def __init__(self, name):
        self.name = name
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
        trace_file = open(self.file, "r")
        trace = None
        test_case = None
        for line in trace_file.readlines():
            test = re.search(r'Entering test case "(.*)"', line, re.M | re.I)
            if test:
                test_name = test.group(1)
                test_case = TestCase(test_name)
                self.tests[test_name] = test_case

            create = re.search(r'CREATE\s*([a-fA-F0-9]*):', line, re.M | re.I)
            if create:
                trace = test_case.add_trace("create", create.group(1))

            call = re.search(r'CALL\s*([a-fA-F0-9]*)\s*->\s*([a-fA-F0-9]*):', line, re.M | re.I)
            if call:
                trace = test_case.add_trace("call", call.group(1))  # + "->" + call.group(2))

            if not create and not call:
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

        trace_file.close()

        print(self.file + ":", len(self.tests), "test-cases.")

        self.ready = True

    def diff(self, analyser):
        if not self.ready:
            self.analyse()
        if not analyser.ready:
            analyser.analyse()

        intersection = set(self.tests.keys()) & set(analyser.tests.keys())
        mismatches = set()

        for test_name in intersection:
            mismatch = False
            mismatch_info = ""
            if len(self.tests[test_name].traces) != len(analyser.tests[test_name].traces):
                mismatches.add(test_name)
                mismatch = True
                mismatch_info += "    trace count are different: " + str(
                    len(self.tests[test_name].traces)) + " != " + str(len(analyser.tests[test_name].traces))
            else:
                for trace_id in range(0, len(self.tests[test_name].traces)):
                    left_trace = self.tests[test_name].traces[trace_id]
                    right_trace = analyser.tests[test_name].traces[trace_id]
                    assert (left_trace.kind == right_trace.kind)
                    if str(left_trace) != str(right_trace):
                        mismatch = True
                        mismatch_info += "    " + str(left_trace) + "\n"
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
                        mismatches.add(test_name)

            if mismatch:
                print(test_name)
                print(mismatch_info)

        print(len(intersection), "test-cases - ", len(mismatches), " mismatches")


def main():
    base_path = os.path.dirname(__file__)
    semantic_trace = TraceAnalyser(base_path + "/semanticTests.trace")
    end_to_end_trace = TraceAnalyser(base_path + "/SolidityEndToEndTest.trace")

    semantic_trace.diff(end_to_end_trace)


if __name__ == "__main__":
    main()
