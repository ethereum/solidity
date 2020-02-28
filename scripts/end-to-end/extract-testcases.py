#!/usr/bin/env python3

import re
import os


class Test:
    def __init__(self, name, content):
        self.name = name
        self.content = content
        self.extractable = True
        self.source = ""
        self.comment = ""
        self.tests = []
        self.alsoViaYul = False

    def analyse(self):
        self.extractable = True
        self.source = self.get_source()
        self.content = self.content.replace(self.source, "", 1)

        for not_allowed in ["m_contractAddress", "m_compiler", "m_evmHost",
                            "m_revertStrings", "m_optimiserSettings", "gasLimit(", "gasPrice(", "blockNumber(",
                            "blockTimestamp(",
                            "numLogTopics(", "logTopic(", "numLogs(", "m_output", "m_sender", "sendMessage(",
                            "m_transactionSuccessful",
                            "solidity::test::CommonOptions::get()", "testContractAgainstCppOnRange(", "testContractAgainstCpp("]:
            self.extractable &= (self.content.find(not_allowed) == -1)

        self.extractable &= self.content.count("compileAndRun") == 1
        self.extractable &= len(self.source) > 0
        self.alsoViaYul = self.content.find("ALSO_VIA_YUL") != -1
        for line in self.content.split("\n"):
            line = line.strip()
            if line.startswith("ABI_CHECK(") & (not line.startswith("ABI_CHECK(callContractFunction")):
                self.extractable = False
        if self.extractable:
            self.get_tests()
        return self.extractable

    def get_source(self):
        for pattern in [r'R"\((.+?)\)";', r'R"YY\((.+?)\)YY";', r'R"ABC\((.+?)\)ABC";', r'R"\*\*\((.+?)\)\*\*";',
                        r'R"T\((.+?)\)T";', r'R"DELIMITER\((.+?)\)DELIMITER";', r'R"XX\((.+?)\)XX";']:
            search = re.search(pattern, self.content, re.MULTILINE | re.DOTALL)

            # check for other sources, test-cases with multiple source definitions will be marked as not extractable
            found = re.findall(pattern, self.content, re.MULTILINE | re.DOTALL)
            if len(found) > 1:
                self.extractable = False

            if search:
                return search.group(1)
        return ""

    def get_tests(self):
        comment = ""
        test_comment = ""
        test_section = False

        for line in self.content.split("\n"):
            line = line.strip()
            if (not test_section) & line.startswith("//"):
                comment += line[2:] + '\n'
            if line.startswith("compileAndRun"):
                test_section = True
            if test_section & line.startswith("//"):
                test_comment += line[2:]
            if line.startswith("ABI_CHECK(") or line.startswith("BOOST_CHECK(") or line.startswith("BOOST_REQUIRE("):
                comment = comment.strip()
                if comment:
                    self.comment = comment
                test_comment = test_comment.strip()
                self.tests.append(self.create_isoltest_call(line, test_comment))
                comment = ""
                test_comment = ""

        # if self.extractable:
        #     print(self.name + ":")
        #     if self.comment:
        #         print("    // " + self.comment.replace("\n", "\n    //"))
        #     for test in self.tests:
        #         print("    " + test)

    def eval_and_correct(self, string):
        if not string:
            return ""
        string = string.replace("u256(", "").replace(")", "").replace("string(", "").replace("encodeArgs(", "").replace(
            "asString(", "").replace("fromHex(", "0x").replace("\"true\"", "true").replace("\"false\"", "false").strip()
        result = ""
        if string.startswith(","):
            string = string[1:].strip()
        if string.startswith("0x") and string.endswith("\""):
            string = string.replace("\"", "")
        if string:
            r = ""
            for arg in string.split(","):
                arg = arg.strip()
                if (arg == "true"):
                    result += "true, "
                elif (arg == "false"):
                    result += "false, "
                else:
                    try:
                        evaluated = eval(arg)
                        if type(evaluated) is int:
                            if string.find("0x") >= 0:
                                r = hex(evaluated)
                            else:
                                r = int(evaluated)
                        elif type(evaluated) is str:
                            r = arg
                        result += str(r) + ", "
                    except:
                        self.extractable = False
                        result += "?, "
            return result[:-2]
        return ""

    def create_call(self, signature, parameters, expectations, comment, withValue=False):
        result = signature
        value = ""
        if withValue:
            if parameters.find(",") == -1:
                value = parameters
                parameters = ''
        if parameters:
            result += ": "
            result += parameters
        if value:
            result += ", " + str(value) + " wei"
        result += " -> "
        if expectations:
            result += expectations
        if comment:
            result += " # " + comment + " #"
        return result

    def create_isoltest_call(self, line, test_comment):
        search = re.search(r'ABI_CHECK\(callContractFunction\(\"(.*)\",((.|\s)*)\), encodeArgs\(((.|\s)*)\)\);', line,
                           re.M | re.I)
        if search:
            result = self.create_call(search.group(1).strip(), self.eval_and_correct(search.group(2)),
                                      self.eval_and_correct(search.group(4)), test_comment)
            return result

        search = re.search(
            r'ABI_CHECK\(callContractFunctionWithValue\(\"(.*)\",((.|\s)*)\), encodeArgs\(((.|\s)*)\)\);', line,
            re.M | re.I)
        if search:
            result = self.create_call(search.group(1).strip(), self.eval_and_correct(search.group(2)),
                                      self.eval_and_correct(search.group(4)), test_comment, True)
            return result

        search = re.search(r'ABI_CHECK\(callContractFunction\(\"(.*)\",((.|\s)*)\), fromHex\("((.|\s)*)"\)\);', line,
                           re.M | re.I)
        if search:
            result = self.create_call(search.group(1).strip(), self.eval_and_correct(search.group(2)),
                                      self.eval_and_correct("0x" + search.group(4)), test_comment)
            return result

        search = re.search(r'ABI_CHECK\(callContractFunction\("(.*)"\), encodeArgs\(((.|\s)*)\)\);', line, re.M | re.I)
        if search:
            result = self.create_call(search.group(1).strip(), "", self.eval_and_correct(search.group(2)),
                                      test_comment)
            return result

        search = re.search(r'ABI_CHECK\(callContractFunction\("(.*)"\), encodeDyn\(((.|\s)*)\)\);', line, re.M | re.I)
        if search:
            result = self.create_call(search.group(1).strip(), "",
                                      self.eval_and_correct(search.group(2)), test_comment)
            return result

        search = re.search(r'ABI_CHECK\(callContractFunction\("(.*)"\), fromHex\("((.|\s)*)"\)\);', line, re.M | re.I)
        if search:
            result = self.create_call(search.group(1).strip(), "",
                                      self.eval_and_correct(search.group(2)), test_comment)
            return result

        search = re.search(r'ABI_CHECK\(callContractFunction\("(.*)"\),\s*bytes{}\);', line, re.M | re.I)
        if search:
            result = self.create_call(search.group(1).strip(), "", "", test_comment)
            return result

        search = re.search(r'ABI_CHECK\(callContractFunction\("(.*)"\,((.|\s)*)\),\s*bytes\(\)\);', line, re.M | re.I)
        if search:
            result = self.create_call(search.group(1).strip(), self.eval_and_correct(search.group(2)), "", test_comment)
            return result

        search = re.search(r'BOOST_CHECK\(callContractFunction\(\"(.*)\"(,\s*.*?)*\)\s*==\s*(.*)\);', line, re.M | re.I)
        if search:
            result = self.create_call(search.group(1).strip(), self.eval_and_correct(search.group(2)),
                                      self.eval_and_correct(search.group(3)), test_comment)
            return result

        search = re.search(r'BOOST_REQUIRE\(callContractFunction\(\"(.*)\"(,\s*.*?)*\)\s*==\sbytes\(\)\);', line, re.M | re.I)
        if search:
            result = self.create_call(search.group(1).strip(), self.eval_and_correct(search.group(2)),
                                      "", test_comment)
            return result

        search = re.search(r'BOOST_REQUIRE\(callContractFunction\(\"(.*)\"(,\s*.*?)*\)\s*==\s*(.*)\);', line, re.M | re.I)
        if search:
            result = self.create_call(search.group(1).strip(), self.eval_and_correct(search.group(2)),
                                      self.eval_and_correct(search.group(3)), test_comment)
            return result


        self.extractable = False
        # if line.endswith("encodeArgs("):
        print(self.name + " -> " + line)
        # print("\n***")
        # print(self.name)
        # print(self.content)
        return "?"

    def extract(self):
        test_file = open(
            os.path.dirname(__file__) + "/extracted/libsolidity/semanticTests/end-to-end/" + self.name + ".sol", "w")
        if self.comment:
            test_file.write("// " + self.comment.replace("\n", "\n    //"))
        test_file.write(self.source + "\n")

        if self.alsoViaYul:
            test_file.write("// ====\n")
            test_file.write("// compileViaYul: also\n");

        test_file.write("// ----\n")
        for test in self.tests:
            test_file.write("// " + test + "\n")
        test_file.write("\n")
        test_file.close()


def main():
    test_name = ""
    test_content = ""
    inside_test = False
    tests = {}

    base_path = os.path.dirname(__file__)
    os.makedirs(base_path + "/extracted/libsolidity/semanticTests/end-to-end", 0o777, True)
    os.makedirs(base_path + "/extracted/libsolidity/ABIJson", 0o777, True)
    os.makedirs(base_path + "/extracted/libsolidity/ASTJSON", 0o777, True)
    os.makedirs(base_path + "/extracted/libsolidity/errorRecoveryTests", 0o777, True)
    os.makedirs(base_path + "/extracted/libsolidity/gasTests", 0o777, True)
    os.makedirs(base_path + "/extracted/libsolidity/smtCheckerTests", 0o777, True)
    os.makedirs(base_path + "/extracted/libsolidity/smtCheckerTestsJSON", 0o777, True)
    os.makedirs(base_path + "/extracted/libsolidity/syntaxTests", 0o777, True)
    os.makedirs(base_path + "/extracted/libyul/ewasmTranslationTests", 0o777, True)
    os.makedirs(base_path + "/extracted/libyul/functionSideEffects", 0o777, True)
    os.makedirs(base_path + "/extracted/libyul/objectCompiler/yulInterpreterTests", 0o777, True)
    os.makedirs(base_path + "/extracted/libyul/yulInterpreterTests", 0o777, True)
    os.makedirs(base_path + "/extracted/libyul/yulOptimizerTests", 0o777, True)
    os.makedirs(base_path + "/extracted/libyul/yulSyntaxTests", 0o777, True)

    cpp_file = open(base_path + "/../../test/libsolidity/SolidityEndToEndTest.cpp", "r")
    for line in cpp_file.readlines():
        test = re.search(r'BOOST_AUTO_TEST_CASE\((.*)\)', line, re.M | re.I)
        if test:
            test_name = test.group(1)
            test_content = ""
        if line == "{\n":
            inside_test = True
        if inside_test:
            test_content += line
        if line == "}\n":
            inside_test = False
            if test_name:
                test = Test(test_name, test_content)
                test.analyse()
                tests[test_name] = test
    cpp_file.close()

    extractable = 0
    not_extractable = 0
    for test in tests:
        if tests[test].extractable:
            tests[test].extract()
            extractable = extractable + 1
        else:
            not_extractable = not_extractable + 1

    print(len(tests), " = ", extractable, "extractable +", not_extractable, "not extractable")


if __name__ == "__main__":
    main()
