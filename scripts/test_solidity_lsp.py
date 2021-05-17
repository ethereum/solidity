#!/usr/bin/env python3.9

# DEPENDENCIES:
#     pip install git+https://github.com/christianparpart/pylspclient.git --user
# WHEN DEVELOPING (editable module):
#     pip install -e /path/to/checkout/pylspclient

import argparse
import os
import subprocess
import threading

from pprint import pprint
from threading import Condition
from deepdiff import DeepDiff

# Requires the one from https://github.com/christianparpart/pylspclient
# Use `pip install -e $PATH_TO_LIB_CHECKOUT --user` for local development & testing
import pylspclient

lsp_types = pylspclient.lsp_structs

SGR_RESET = '\033[m'
SGR_TEST_BEGIN = '\033[1;33m'
SGR_STATUS_OKAY = '\033[1;32m'
SGR_STATUS_FAIL = '\033[1;31m'
SGR_INSPECT = '\033[1;35m'

TEST_NAME = 'test_definition'

def dprint(text: str):
    print(SGR_INSPECT + "-- " + text + ":" + SGR_RESET)

def dinspect(text, obj):
    dprint(text)
    if not obj is None:
        pprint(obj)

class ReadPipe(threading.Thread):
    """
        Used to link (solc) process stdio.
    """
    def __init__(self, pipe):
        threading.Thread.__init__(self)
        self.pipe = pipe

    def run(self):
        dprint("ReadPipe: starting")
        line = self.pipe.readline().decode('utf-8')
        while line:
            print(line)
            #print("\033[1;42m{}\033[m\n".format(line))
            line = self.pipe.readline().decode('utf-8')

SOLIDITY_LANGUAGE_ID = "solidity" # lsp_types.LANGUAGE_IDENTIFIER.C

LSP_CLIENT_CAPS = {
        'textDocument': {'codeAction': {'dynamicRegistration': True},
        'codeLens': {'dynamicRegistration': True},
        'colorProvider': {'dynamicRegistration': True},
        'completion': {'completionItem': {'commitCharactersSupport': True,
            'documentationFormat': ['markdown', 'plaintext'],
            'snippetSupport': True},
        'completionItemKind': {'valueSet': [
            1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
            17, 18, 19, 20, 21, 22, 23, 24, 25
        ]},
        'contextSupport': True,
        'dynamicRegistration': True},
        'definition': {'dynamicRegistration': True},
        'documentHighlight': {'dynamicRegistration': True},
        'documentLink': {'dynamicRegistration': True},
        'documentSymbol': {
            'dynamicRegistration': True,
            'symbolKind': {'valueSet': [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                17, 18, 19, 20, 21, 22, 23, 24, 25, 26
            ]}
        },
        'formatting': {'dynamicRegistration': True},
        'hover': {'contentFormat': ['markdown', 'plaintext'],
        'dynamicRegistration': True},
        'implementation': {'dynamicRegistration': True},
        'onTypeFormatting': {'dynamicRegistration': True},
        'publishDiagnostics': {'relatedInformation': True},
        'rangeFormatting': {'dynamicRegistration': True},
        'references': {'dynamicRegistration': True},
        'rename': {'dynamicRegistration': True},
        'signatureHelp': {'dynamicRegistration': True,
        'signatureInformation': {'documentationFormat': ['markdown', 'plaintext']}},
        'synchronization': {'didSave': True,
        'dynamicRegistration': True,
        'willSave': True,
        'willSaveWaitUntil': True},
        'typeDefinition': {'dynamicRegistration': True}},
        'workspace': {'applyEdit': True,
        'configuration': True,
        'didChangeConfiguration': {'dynamicRegistration': True},
        'didChangeWatchedFiles': {'dynamicRegistration': True},
        'executeCommand': {'dynamicRegistration': True},
        'symbol': {
            'dynamicRegistration': True,
            'symbolKind': {'valueSet': [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                                        13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
                                        23, 24, 25, 26 ]}
        },
        'workspaceEdit': {'documentChanges': True},
        'workspaceFolders': True}
    }

class SolcInstance:
    """
    Manages the solc executable instance and provides the handle to communicate with it
    """

    process: subprocess.Popen
    endpoint: pylspclient.LspEndpoint
    client: pylspclient.LspClient
    read_pipe: ReadPipe
    diagnostics_cond: Condition
    #published_diagnostics: object

    def __init__(self, _solc_path: str) -> None:
        self.solc_path = _solc_path
        self.published_diagnostics = []
        self.client = pylspclient.LspClient(None)
        self.diagnostics_cond = Condition()

    def __enter__(self):
        dprint(f"Starting solc LSP instance: {self.solc_path}")
        self.process = subprocess.Popen(
            [self.solc_path, "--lsp"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        self.read_pipe = ReadPipe(self.process.stderr)
        self.read_pipe.start()
        self.endpoint = pylspclient.LspEndpoint(
            json_rpc_endpoint=pylspclient.JsonRpcEndpoint(
                self.process.stdin,
                self.process.stdout
            ),
            notify_callbacks={
                'textDocument/publishDiagnostics': self.on_publish_diagnostics
            }
        )
        self.client = pylspclient.LspClient(self.endpoint)
        return self

    def __exit__(self, _exception_type, _exception_value, _traceback) -> None:
        dprint("Stopping solc instance.")
        self.client.shutdown()
        self.client.exit()
        self.read_pipe.join()

    def on_publish_diagnostics(self, _diagnostics) -> None:
        dprint("Receiving published diagnostics:")
        pprint(_diagnostics)
        self.published_diagnostics.append(_diagnostics)
        self.diagnostics_cond.acquire()
        self.diagnostics_cond.notify()
        self.diagnostics_cond.release()

class SolcTests:
    def __init__(self, _client: SolcInstance, _project_root_dir: str):
        self.solc = _client
        self.project_root_dir = _project_root_dir
        self.project_root_uri = 'file://' + self.project_root_dir
        self.tests = 0
        dprint("root dir: {self.project_root_dir}")

    # {{{ helpers
    def get_test_file_path(self, _test_case_name):
        return f"{self.project_root_dir}/{_test_case_name}.sol"

    def get_test_file_uri(self, _test_case_name):
        return "file://" + self.get_test_file_path(_test_case_name)

    def get_test_file_contents(self, _test_case_name):
        return open(self.get_test_file_path(_test_case_name,), mode="r", encoding="utf-8").read()

    def lsp_open_file(self, _test_case_name):
        """ Opens file for given test case. """
        version = 1
        file_uri = self.get_test_file_uri(_test_case_name)
        file_contents = self.get_test_file_contents(_test_case_name)
        self.solc.client.didOpen(lsp_types.TextDocumentItem(
            file_uri, SOLIDITY_LANGUAGE_ID, version, file_contents
        ))

    def lsp_open_file_and_wait_for_diagnostics(self, _test_case_name):
        """
        Opens file for given test case and waits for diagnostics to be published.
        """
        self.solc.diagnostics_cond.acquire()
        self.lsp_open_file(_test_case_name)
        self.solc.diagnostics_cond.wait_for(
            predicate=lambda: len(self.solc.published_diagnostics) != 0,
            timeout=2.0
        )
        self.solc.diagnostics_cond.release()

    def expect(self, _cond: bool, _description: str) -> None:
        self.tests = self.tests + 1
        prefix = f"[{self.tests}] {SGR_TEST_BEGIN}{_description}{SGR_RESET} : "
        if _cond:
            print(prefix + SGR_STATUS_OKAY + 'OK' + SGR_RESET)
        else:
            print(prefix + SGR_STATUS_FAIL + 'FAILED' + SGR_RESET)
            raise RuntimeError("Expectation failed.")

    def expect_equal(self, _description: str, _actual, _expected) -> None:
        self.tests = self.tests + 1
        prefix = f"[{self.tests}] {SGR_TEST_BEGIN}{_description}: "
        diff = DeepDiff(_actual, _expected)
        if len(diff) == 0:
            print(prefix + SGR_STATUS_OKAY + 'OK' + SGR_RESET)
            return

        print(prefix + SGR_STATUS_FAIL + 'FAILED' + SGR_RESET)
        pprint(diff)
        raise RuntimeError('Expectation failed.')

    # }}}

    # {{{ actual tests
    def run(self):
        self.open_files_and_test_publish_diagnostics()
        self.test_definition()
        self.test_documentHighlight()
        # self.test_hover()
        # self.test_implementation()
        # self.test_references()
        # self.test_signatureHelp()
        # self.test_semanticTokensFull()

    def extract_test_file_name(self, _uri: str):
        """
        Extracts the project-root URI prefix from the URI.
        """
        subLength = len(self.project_root_uri)
        return _uri[subLength:]

    def open_files_and_test_publish_diagnostics(self):
        self.lsp_open_file_and_wait_for_diagnostics(TEST_NAME)

        # should have received one published_diagnostics notification
        dprint("len: {len(self.solc.published_diagnostics)}")
        self.expect(len(self.solc.published_diagnostics) == 1, "one published_diagnostics message")
        published_diagnostics = self.solc.published_diagnostics[0]

        self.expect(published_diagnostics['uri'] == self.get_test_file_uri(TEST_NAME),
            'diagnostic: uri')

        # containing one single diagnostics report
        diagnostics = published_diagnostics['diagnostics']
        self.expect(len(diagnostics) == 1, "one diagnostics")
        diagnostic = diagnostics[0]
        self.expect(diagnostic['code'] == 3805, 'diagnostic: pre-release compiler')
        self.expect_equal(
            'check range',
            diagnostic['range'],
            {'end': {'character': 0, 'line': 0}, 'start': {'character': 0, 'line': 0}}
        )

    def test_definition(self):
        """
        Tests goto-definition. The following tokens can be used to jump from:
        """

        self.solc.published_diagnostics.clear()

        # LHS enum variable in assignment: `weather`
        result = self.solc.client.definition(
                lsp_types.TextDocumentIdentifier(self.get_test_file_uri(TEST_NAME)),
                lsp_types.Position(23, 9)) # line/col numbers are 0-based
        self.expect(len(result) == 1, "only one definition returned")
        self.expect(result[0].range == lsp_types.Range(lsp_types.Position(19, 16),
                                                       lsp_types.Position(19, 23)), "range check")

        # Test on return parameter symbol: `result` at 35:9 (begin of identifier)
        result = self.solc.client.definition(
                lsp_types.TextDocumentIdentifier(self.get_test_file_uri(TEST_NAME)),
                lsp_types.Position(24, 8))

        # Test goto-def of a function-parameter.
        result = self.solc.client.definition(
                lsp_types.TextDocumentIdentifier(self.get_test_file_uri(TEST_NAME)),
                lsp_types.Position(24, 17))

        # Test on function parameter symbol
        # Test on enum type symbol in expression
        # Test on enum value symbol in expression
        # Test on import statement to jump to imported file

    def test_documentHighlight(self):
        """
        Tests symbolic hover-hints, that is, highlighting all other symbols
        that are semantically the same.

        - hovering a variable symbol will highlight all occurrences of that variable.
        - hovering a function symbol will hover all other calls to it as well
          as the function definition's function name itself.
        - hovering an enum value will highlight all other uses of that enum value,
          including the name of the definition of that enum value.
        - hovering an enum type will highlight all other uses of that enum type,
          including the name of the definition of that enum type.
        - anything else will reply with an empty set.
        """

        # variable
        reply = self.solc.client.documentHighlight(
            lsp_types.TextDocumentIdentifier(self.get_test_file_uri(TEST_NAME)),
            lsp_types.Position(23, 9) # line/col numbers are 0-based
        )
        self.expect(len(reply) == 2, "2 highlights")
        self.expect(reply[0].kind == lsp_types.DocumentHighlightKind.Text, "kind")
        self.expect(reply[0].range == lsp_types.Range(lsp_types.Position(19, 16),
                                                      lsp_types.Position(19, 23)), "range check")
        self.expect(reply[1].kind == lsp_types.DocumentHighlightKind.Text, "kind")
        self.expect(reply[1].range == lsp_types.Range(lsp_types.Position(23, 8),
                                                      lsp_types.Position(23, 15)), "range check")

        # enum type: Weather, line 24, col 19 .. 25
        reply = self.solc.client.documentHighlight(
            lsp_types.TextDocumentIdentifier(self.get_test_file_uri(TEST_NAME)),
            lsp_types.Position(23, 18) # line/col numbers are 0-based
        )
        ENUM_TYPE_HIGHLIGHTS = [
            { 'kind': lsp_types.DocumentHighlightKind.Text, 'line':  6, 'from':  5, 'to': 12 },
            { 'kind': lsp_types.DocumentHighlightKind.Text, 'line': 14, 'from':  4, 'to': 11 },
            { 'kind': lsp_types.DocumentHighlightKind.Text, 'line': 14, 'from': 27, 'to': 34 },
            { 'kind': lsp_types.DocumentHighlightKind.Text, 'line': 19, 'from':  8, 'to': 15 },
            { 'kind': lsp_types.DocumentHighlightKind.Text, 'line': 19, 'from': 26, 'to': 33 },
            { 'kind': lsp_types.DocumentHighlightKind.Text, 'line': 23, 'from': 18, 'to': 25 }
        ]
        self.expect(len(reply) == len(ENUM_TYPE_HIGHLIGHTS), f"expect {len(ENUM_TYPE_HIGHLIGHTS)} highlights")
        for i in range(0, len(reply)):
            self.expect(reply[i].kind == ENUM_TYPE_HIGHLIGHTS[i]['kind'], "check kind")
            self.expect(
                reply[i].range == lsp_types.Range(
                    lsp_types.Position(
                        ENUM_TYPE_HIGHLIGHTS[i]['line'],
                        ENUM_TYPE_HIGHLIGHTS[i]['from']
                    ),
                    lsp_types.Position(
                        ENUM_TYPE_HIGHLIGHTS[i]['line'],
                        ENUM_TYPE_HIGHLIGHTS[i]['to']
                    )
                ),
                "range check"
            )


    def test_references(self):
        # Shows all references of given symbol def
        pass

    def test_hover(self):
        pass

    # }}}

class SolidityLSPTestSuite: # {{{
    def __init__(self):
        self.project_root_dir = ''
        self.solc_path = ''

    def main(self):
        solc_path, project_root_dir = self.parse_args_and_prepare()

        with SolcInstance(solc_path) as solc:
            project_root_uri = 'file://' + project_root_dir
            workspace_folders = [ {'name': 'solidity-lsp', 'uri': project_root_uri} ]
            traceServer = 'off'
            solc.client.initialize(solc.process.pid, None, project_root_uri,
                                   None, LSP_CLIENT_CAPS, traceServer,
                                   workspace_folders)
            solc.client.initialized()
            tests = SolcTests(solc, project_root_dir)
            tests.run()

    def parse_args_and_prepare(self):
        """
        Parses CLI args and retruns tuple of path to solc executable
        and path to solidity-project root dir.
        """
        parser = argparse.ArgumentParser(description='Solidity LSP Test suite')
        parser.add_argument(
            'solc_path',
            type=str,
            default="/home/trapni/work/solidity/build/solc/solc",
            help='Path to solc binary to test against',
            nargs="?"
        )
        parser.add_argument(
            'project_root_dir',
            type=str,
            default=f"{os.path.dirname(os.path.realpath(__file__))}/..",
            help='Path to Solidity project\'s root directory (must be fully qualified).',
            nargs="?"
        )
        args = parser.parse_args()
        project_root_dir = os.path.realpath(args.project_root_dir) + '/test/libsolidity/lsp'
        return [args.solc_path, project_root_dir]
    # }}}

if __name__ == "__main__":
    suite = SolidityLSPTestSuite()
    suite.main()
