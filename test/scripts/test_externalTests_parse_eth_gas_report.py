#!/usr/bin/env python3

from dataclasses import asdict
import unittest

from textwrap import dedent

from unittest_helpers import FIXTURE_DIR, load_fixture

# NOTE: This test file file only works with scripts/ added to PYTHONPATH so pylint can't find the imports
# pragma pylint: disable=import-error
from externalTests.parse_eth_gas_report import parse_report, ReportParsingError, ReportValidationError
# pragma pylint: enable=import-error

ETH_GAS_REPORT_GNOSIS_RST_PATH = FIXTURE_DIR / 'eth_gas_report_gnosis.rst'
ETH_GAS_REPORT_GNOSIS_RST_CONTENT = load_fixture(ETH_GAS_REPORT_GNOSIS_RST_PATH)


class TestEthGasReport(unittest.TestCase):
    def setUp(self):
        self.maxDiff = 10000

    def test_parse_report(self):
        parsed_report = parse_report(ETH_GAS_REPORT_GNOSIS_RST_CONTENT)

        expected_report = {
            'solc_version': '0.8.10',
            'optimize': True,
            'runs': 200,
            'block_limit': 100000000,
            'total_method_gas': 57826 * 6 + 53900 * 2 + 51567 * 8 + 94816 * 85 + 201944 * 49 + 105568 * 52,
            'total_deployment_gas': 283516 + 525869 + 733462,
            'contracts': {
                'DelegateCallTransactionGuard': {
                    'total_method_gas': 0,
                    'min_deployment_gas': 283510,
                    'max_deployment_gas': 283522,
                    'avg_deployment_gas': 283516,
                    'methods': None,
                },
                'ERC1155Token': {
                    'total_method_gas': 57826 * 6 + 53900 * 2,
                    'min_deployment_gas': None,
                    'max_deployment_gas': None,
                    'avg_deployment_gas': 525869,
                    'methods': {
                        'mint(address,uint256,uint256,bytes)': {
                            'total_gas': 57826 * 6,
                            'min_gas': 47934,
                            'max_gas': 59804,
                            'avg_gas': 57826,
                            'call_count': 6
                        },
                        'safeTransferFrom(address,address,uint256,uint256,bytes)': {
                            'total_gas': 53900 * 2,
                            'min_gas': 53900,
                            'max_gas': 53900,
                            'avg_gas': 53900,
                            'call_count': 2,
                        },
                    },
                },
                'ERC20Token': {
                    'total_method_gas': 51567 * 8,
                    'min_deployment_gas': None,
                    'max_deployment_gas': None,
                    'avg_deployment_gas': 733462,
                    'methods': {
                        'transfer(address,uint256)': {
                            'total_gas': 51567 * 8,
                            'min_gas': 51567,
                            'max_gas': 51567,
                            'avg_gas': 51567,
                            'call_count': 8,
                        },
                    },
                },
                'GnosisSafe': {
                    'total_method_gas': 94816 * 85 + 201944 * 49,
                    'min_deployment_gas': None,
                    'max_deployment_gas': None,
                    'avg_deployment_gas': None,
                    'methods': {
                        'execTransaction(address,uint256,bytes,uint8,uint256,uint256,uint256,address,address,bytes)': {
                            'total_gas': 94816 * 85,
                            'min_gas': 59563,
                            'max_gas': 151736,
                            'avg_gas': 94816,
                            'call_count': 85,
                        },
                        'setup(address[],uint256,address,bytes,address,address,uint256,address)': {
                            'total_gas': 201944 * 49,
                            'min_gas': 167642,
                            'max_gas': 263690,
                            'avg_gas': 201944,
                            'call_count': 49,
                        },
                    },
                },
                'GnosisSafeProxyFactory': {
                    'total_method_gas': 105568 * 52,
                    'min_deployment_gas': None,
                    'max_deployment_gas': None,
                    'avg_deployment_gas': None,
                    'methods': {
                        'createProxy(address,bytes)': {
                            'total_gas': 105568 * 52,
                            'min_gas': 105568,
                            'max_gas': 105580,
                            'avg_gas': 105568,
                            'call_count': 52,
                        },
                    },
                },
            }
        }
        self.assertEqual(asdict(parsed_report), expected_report)

    def test_parse_report_should_fail_if_report_is_empty(self):
        text_report = ""
        with self.assertRaises(ReportValidationError) as manager:
            parse_report(text_report)
        self.assertEqual(str(manager.exception), "Report header not found.")

    def test_parse_report_should_fail_if_report_has_no_header(self):
        text_report = dedent("""
            | Methods                                            |
            | ERC1155Token · mint() · 1 · 3 · 2 ·          6 · - |
            | Deployments           ·           · % of limit ·   │
            | ERC1155Token          · - · - · 5 ·        1 % · - |
        """).strip('\n')
        with self.assertRaises(ReportValidationError) as manager:
            parse_report(text_report)
        self.assertEqual(str(manager.exception), "Report header not found.")

    def test_parse_report_should_fail_if_data_rows_have_no_headers(self):
        text_report = dedent("""
            | ERC1155Token · mint() · 1 · 3 · 2 · 6 · - |
        """).strip('\n')
        expected_message = dedent("""
            Parsing error on line 1: Found data row without a section header.
            | ERC1155Token | mint() | 1 | 3 | 2 | 6 | - |
        """).strip('\n')

        with self.assertRaises(ReportParsingError) as manager:
            parse_report(text_report)
        self.assertEqual(str(manager.exception), expected_message)

    def test_parse_report_should_fail_if_report_has_more_than_one_header(self):
        text_report = dedent("""
            | Solc version: 0.8.10 · Optimizer enabled: true  · Runs: 200 · Block limit: 100000000 gas |
            | Solc version: 0.8.9  · Optimizer enabled: false · Runs: 111 · Block limit: 999999999 gas |
        """).strip('\n')
        expected_message = dedent("""
            Parsing error on line 2: Duplicate report header.
            | Solc version: 0.8.9  | Optimizer enabled: false | Runs: 111 | Block limit: 999999999 gas |
        """).strip('\n')

        with self.assertRaises(ReportParsingError) as manager:
            parse_report(text_report)
        self.assertEqual(str(manager.exception), expected_message)

    def test_parse_report_should_fail_if_row_matching_same_method_call_appears_twice(self):
        text_report = dedent("""
            | Methods                                               |
            | ERC1155Token · mint() · 47934 · 59804 · 57826 · 6 · - |
            | ERC1155Token · mint() · 11111 · 22222 · 33333 · 4 · - |
        """).strip('\n')
        expected_message = dedent("""
            Parsing error on line 3: Duplicate method row for 'ERC1155Token.mint()'.
            | ERC1155Token | mint() | 11111 | 22222 | 33333 | 4 | - |
        """).strip('\n')

        with self.assertRaises(ReportParsingError) as manager:
            parse_report(text_report)
        self.assertEqual(str(manager.exception), expected_message)

    def test_parse_report_should_fail_if_row_matching_same_contract_deployment_appears_twice(self):
        text_report = dedent("""
            | Deployments          ·        · % of limit ·   │
            | ERC1155Token · - · - · 525869 ·      0.5 % · - |
            | ERC1155Token · - · - · 111111 ·      0.6 % · - |
        """).strip('\n')
        expected_message = dedent("""
            Parsing error on line 3: Duplicate contract deployment row for 'ERC1155Token'.
            | ERC1155Token | - | - | 111111 |      0.6 % | - |
        """).strip('\n')

        with self.assertRaises(ReportParsingError) as manager:
            parse_report(text_report)
        self.assertEqual(str(manager.exception), expected_message)

    def test_parse_report_should_fail_if_method_row_appears_under_deployments_header(self):
        text_report = dedent("""
            | Deployments           ·                       · % of limit ·   │
            | ERC1155Token · mint() · 47934 · 59804 · 57826 ·          6 · - |
        """).strip('\n')
        expected_message = dedent("""
            Parsing error on line 2: Expected a table row with deployment details.
            | ERC1155Token | mint() | 47934 | 59804 | 57826 |          6 | - |
        """).strip('\n')

        with self.assertRaises(ReportParsingError) as manager:
            parse_report(text_report)
        self.assertEqual(str(manager.exception), expected_message)

    def test_parse_report_should_fail_if_deployment_row_appears_under_methods_header(self):
        text_report = dedent("""
            | Methods                               |
            | ERC1155Token · - · - · 525869 · 5 · - |
        """).strip('\n')
        expected_message = dedent("""
            Parsing error on line 2: Expected a table row with method details.
            | ERC1155Token | - | - | 525869 | 5 | - |
        """).strip('\n')

        with self.assertRaises(ReportParsingError) as manager:
            parse_report(text_report)
        self.assertEqual(str(manager.exception), expected_message)
