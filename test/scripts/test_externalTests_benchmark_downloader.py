#!/usr/bin/env python3

from pathlib import Path
from unittest import TestCase
from unittest.mock import call, Mock, patch

# NOTE: This test file file only works with scripts/ added to PYTHONPATH so pylint can't find the imports
# pragma pylint: disable=import-error
from externalTests.download_benchmarks import download_benchmarks
# pragma pylint: enable=import-error


def _git_run_command_mock(command):
    if command == ['git', 'symbolic-ref', 'HEAD', '--short']:
        return 'benchmark-downloader'

    if len(command) == 4 and command[:3] == ['git', 'rev-parse', '--verify']:
        ref = command[3]
        if ref == 'HEAD':
            ref = 'benchmark-downloader'

        if ref == 'benchmark-downloader':
            return 'fa1ddc6f412100d531f6d3a77008c73b474692d6'

        if ref == 'develop':
            return '43f29c00da02e19ff10d43f7eb6955d627c57728'

    raise RuntimeError(
        "The test tried to run an unexpected git command.\n"
        f"command: {command}\n"
        "If you have updated the code, please remember to add matching command fixtures above."
    )

def _requests_get_mock(url, params):
    response_mock = Mock()

    if url == 'https://api.github.com/repos/ethereum/solidity/pulls/12818':
        response_mock.json.return_value = {
            "head": {
                "ref": "benchmark-downloader",
                "sha": "fa1ddc6f412100d531f6d3a77008c73b474692d6",
            },
            "base": {
                "ref": "develop",
                "sha": "43f29c00da02e19ff10d43f7eb6955d627c57728",
            },
        }
        return response_mock

    if (
        url == 'https://circleci.com/api/v2/project/gh/ethereum/solidity/pipeline' and
        params.get('branch') == 'develop'
    ):
        response_mock.json.return_value = {
            "next_page_token": None,
            "items": [
                {
                    "id": "3b15a41f-6933-4a35-9823-08ebb1ff9336",
                    "created_at": "2022-03-23T00:10:31.659Z",
                    "trigger": {"type": "schedule"},
                    "vcs": {
                        "revision": "43f29c00da02e19ff10d43f7eb6955d627c57728",
                        "branch": "develop"
                    },
                },
                {
                    "id": "f9036a2d-be2b-4315-bd57-4d35b87502d2",
                    "created_at": "2022-03-22T00:10:30.304Z",
                    "trigger": {"type": "webhook"},
                    "vcs": {
                        "revision": "43f29c00da02e19ff10d43f7eb6955d627c57728",
                        "branch": "develop"
                    },
                },
                {
                    "id": "1d389e7c-b7dc-4d4d-9e58-c21ae48901a5",
                    "created_at": "2022-03-21T00:10:30.579Z",
                    "trigger": {"type": "schedule"},
                    "vcs": {
                        "revision": "430ecb6e16c346005315dbdd3edf3c3e64e9b1d8",
                        "branch": "develop"
                    },
                },
                {
                    "id": "7185a3f6-6338-4c2c-952d-4c30e7561e61",
                    "created_at": "2022-03-21T12:54:41.817Z",
                    "trigger": {"type": "webhook"},
                    "vcs": {
                        "revision": "43f29c00da02e19ff10d43f7eb6955d627c57728",
                        "branch": "develop"
                    }
                },
            ]
        }
        return response_mock

    if (
        url == 'https://circleci.com/api/v2/project/gh/ethereum/solidity/pipeline' and
        params.get('branch') == 'benchmark-downloader'
    ):
        response_mock.json.return_value = {
            "next_page_token": None,
            "items": [
                {
                    "id": "9af60346-a6b9-41b9-8a16-16ccf8996373",
                    "created_at": "2022-03-23T10:11:34.683Z",
                    "trigger": {"type": "webhook"},
                    "vcs": {
                        "revision": "fa1ddc6f412100d531f6d3a77008c73b474692d6",
                        "branch": "benchmark-downloader"
                    }
                }
            ]
        }
        return response_mock

    if (url in [
        # To reduce the number of fixtures, let's use this workflow for multiple pipelines.
        # This would not be the case in practice.
        'https://circleci.com/api/v2/pipeline/f9036a2d-be2b-4315-bd57-4d35b87502d2/workflow',
        'https://circleci.com/api/v2/pipeline/9af60346-a6b9-41b9-8a16-16ccf8996373/workflow'
    ]):
        response_mock.json.return_value = {
            "next_page_token": None,
            "items": [
                {
                    "id": "7a54e9cc-513d-4134-afdb-db62ab8146e5",
                    "created_at": "2022-03-21T12:54:42Z",
                }
            ]
        }
        return response_mock

    if url == 'https://circleci.com/api/v2/workflow/7a54e9cc-513d-4134-afdb-db62ab8146e5/job':
        response_mock.json.return_value = {
            "next_page_token": None,
            "items": [
                {
                    "job_number": 1017975,
                    "name": "chk_coding_style",
                    "status": "success",
                },
                {
                    "job_number": 1017969,
                    "name": "b_ubu",
                    "status": "success",
                },
                {
                    "job_number": 1018023,
                    "name": "c_ext_benchmarks",
                    "status": "success",
                },
            ]
        }
        return response_mock

    if url == 'https://circleci.com/api/v2/project/gh/ethereum/solidity/1018023/artifacts':
        response_mock.json.return_value = {
            "next_page_token": None,
            "items": [
                {
                    "path": "reports/externalTests/all-benchmarks.json",
                    "url": "https://circle-artifacts.com/0/reports/externalTests/all-benchmarks.json"
                },
                {
                    "path": "reports/externalTests/summarized-benchmarks.json",
                    "url": "https://circle-artifacts.com/0/reports/externalTests/summarized-benchmarks.json"
                }
            ]
        }
        return response_mock

    raise RuntimeError(
        "The test tried to perform an unexpected GET request.\n"
        f"URL: {url}\n" +
        (f"query: {params}\n" if len(params) > 0 else "") +
        "If you have updated the code, please remember to add matching response fixtures above."
    )

class TestBenchmarkDownloader(TestCase):
    def setUp(self):
        self.maxDiff = 10000

    @staticmethod
    @patch('externalTests.download_benchmarks.download_file')
    @patch('requests.get', _requests_get_mock)
    @patch('common.git_helpers.run_git_command',_git_run_command_mock)
    def test_download_benchmarks(download_file_mock):
        download_benchmarks(None, None, None, silent=True)
        download_file_mock.assert_has_calls([
            call(
                'https://circle-artifacts.com/0/reports/externalTests/summarized-benchmarks.json',
                Path('summarized-benchmarks-benchmark-downloader-fa1ddc6f.json'),
                False
            ),
            call(
                'https://circle-artifacts.com/0/reports/externalTests/all-benchmarks.json',
                Path('all-benchmarks-benchmark-downloader-fa1ddc6f.json'),
                False
            ),
        ])

    @staticmethod
    @patch('externalTests.download_benchmarks.download_file')
    @patch('requests.get', _requests_get_mock)
    @patch('common.git_helpers.run_git_command',_git_run_command_mock)
    def test_download_benchmarks_branch(download_file_mock):
        download_benchmarks('develop', None, None, silent=True)
        download_file_mock.assert_has_calls([
            call(
                'https://circle-artifacts.com/0/reports/externalTests/summarized-benchmarks.json',
                Path('summarized-benchmarks-develop-43f29c00.json'),
                False
            ),
            call(
                'https://circle-artifacts.com/0/reports/externalTests/all-benchmarks.json',
                Path('all-benchmarks-develop-43f29c00.json'),
                False
            ),
        ])

    @staticmethod
    @patch('externalTests.download_benchmarks.download_file')
    @patch('requests.get', _requests_get_mock)
    @patch('common.git_helpers.run_git_command',_git_run_command_mock)
    def test_download_benchmarks_pr(download_file_mock):
        download_benchmarks(None, 12818, None, silent=True)
        download_file_mock.assert_has_calls([
            call(
                'https://circle-artifacts.com/0/reports/externalTests/summarized-benchmarks.json',
                Path('summarized-benchmarks-benchmark-downloader-fa1ddc6f.json'),
                False
            ),
            call(
                'https://circle-artifacts.com/0/reports/externalTests/all-benchmarks.json',
                Path('all-benchmarks-benchmark-downloader-fa1ddc6f.json'),
                False
            ),
        ])

    @staticmethod
    @patch('externalTests.download_benchmarks.download_file')
    @patch('requests.get', _requests_get_mock)
    @patch('common.git_helpers.run_git_command',_git_run_command_mock)
    def test_download_benchmarks_base_of_pr(download_file_mock):
        download_benchmarks(None, None, 12818, silent=True)
        download_file_mock.assert_has_calls([
            call(
                'https://circle-artifacts.com/0/reports/externalTests/summarized-benchmarks.json',
                Path('summarized-benchmarks-develop-43f29c00.json'),
                False
            ),
            call(
                'https://circle-artifacts.com/0/reports/externalTests/all-benchmarks.json',
                Path('all-benchmarks-develop-43f29c00.json'),
                False
            ),
        ])
