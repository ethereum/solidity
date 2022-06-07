#!/usr/bin/env python3

from argparse import ArgumentParser, Namespace
from enum import Enum, unique
from pathlib import Path
from typing import Mapping, Optional
import sys

import requests

# Our scripts/ is not a proper Python package so we need to modify PYTHONPATH to import from it
# pragma pylint: disable=import-error,wrong-import-position
SCRIPTS_DIR = Path(__file__).parent.parent
sys.path.insert(0, str(SCRIPTS_DIR))

from common.git_helpers import git_current_branch, git_commit_hash
from common.rest_api_helpers import APIHelperError, JobNotSuccessful, CircleCI, Github, download_file
# pragma pylint: enable=import-error,wrong-import-position


@unique
class Status(Enum):
    OK = 0            # Benchmarks downloaded successfully
    ERROR = 1         # Error in the script, bad API response, unexpected data, etc.
    NO_BENCHMARK = 2  # Benchmark collector job did not finish successfully and/or benchmark artifacts are missing.
    PENDING = 3       # Benchmark collector job has not finished yet.


def process_commandline() -> Namespace:
    script_description = (
        "Downloads benchmark results attached as artifacts to the c_ext_benchmarks job on CircleCI. "
        "If no options are specified, downloads results for the currently checked out git branch."
    )

    parser = ArgumentParser(description=script_description)

    target_definition = parser.add_mutually_exclusive_group()
    target_definition.add_argument(
        '--branch',
        dest='branch',
        help="Git branch that the job ran on.",
    )
    target_definition.add_argument(
        '--pr',
        dest='pull_request_id',
        type=int,
        help="Github PR ID that the job ran on.",
    )
    target_definition.add_argument(
        '--base-of-pr',
        dest='base_of_pr',
        type=int,
        help="ID of a Github PR that's based on top of the branch we're interested in."
    )

    parser.add_argument(
        '--any-commit',
        dest='ignore_commit_hash',
        default=False,
        action='store_true',
        help="Include pipelines that ran on a different commit as long as branch/PR matches."
    )
    parser.add_argument(
        '--overwrite',
        dest='overwrite',
        default=False,
        action='store_true',
        help="If artifacts already exist on disk, overwrite them.",
    )
    parser.add_argument(
        '--debug-requests',
        dest='debug_requests',
        default=False,
        action='store_true',
        help="Print detailed info about performed API requests and received responses.",
    )

    return parser.parse_args()


def download_benchmark_artifact(
    artifacts: Mapping[str, dict],
    benchmark_name: str,
    branch: str,
    commit_hash: str,
    overwrite: bool,
    silent: bool = False
) -> bool:
    if not silent:
        print(f"Downloading artifact: {benchmark_name}-{branch}-{commit_hash[:8]}.json.")

    artifact_path = f'reports/externalTests/{benchmark_name}.json'

    if artifact_path not in artifacts:
        if not silent:
            print(f"Missing artifact: {artifact_path}.")
        return False

    download_file(
        artifacts[artifact_path]['url'],
        Path(f'{benchmark_name}-{branch}-{commit_hash[:8]}.json'),
        overwrite,
    )

    return True


def download_benchmarks(
    branch: Optional[str],
    pull_request_id: Optional[int],
    base_of_pr: Optional[int],
    ignore_commit_hash: bool = False,
    overwrite: bool = False,
    debug_requests: bool = False,
    silent: bool = False,
) -> Status:
    github = Github('ethereum/solidity', debug_requests)
    circleci = CircleCI('ethereum/solidity', debug_requests)

    expected_commit_hash = None
    if branch is None and pull_request_id is None and base_of_pr is None:
        branch = git_current_branch()
        expected_commit_hash = git_commit_hash()
    elif branch is not None:
        expected_commit_hash = git_commit_hash(branch)
    elif pull_request_id is not None:
        pr_info = github.pull_request(pull_request_id)
        branch = pr_info['head']['ref']
        expected_commit_hash = pr_info['head']['sha']
    elif base_of_pr is not None:
        pr_info = github.pull_request(base_of_pr)
        branch = pr_info['base']['ref']
        expected_commit_hash = pr_info['base']['sha']

    if not silent:
        print(
            f"Looking for pipelines that ran on branch {branch}" +
            (f", commit {expected_commit_hash}." if not ignore_commit_hash else " (any commit).")
        )

    pipeline = circleci.latest_item(circleci.pipelines(
        branch,
        expected_commit_hash if not ignore_commit_hash else None,
        # Skip nightly workflows. They don't have the c_ext_benchmarks job and even if they did,
        # they would likely be running a different set of external tests.
        excluded_trigger_types=['schedule'],
    ))
    if pipeline is None:
        raise RuntimeError("No matching pipelines found.")

    actual_commit_hash = pipeline['vcs']['revision']
    workflow_id = circleci.latest_item(circleci.workflows(pipeline['id']))['id']
    benchmark_collector_job = circleci.job(workflow_id, 'c_ext_benchmarks', require_success=True)

    artifacts = circleci.artifacts(int(benchmark_collector_job['job_number']))

    got_summary = download_benchmark_artifact(artifacts, 'summarized-benchmarks', branch, actual_commit_hash, overwrite, silent)
    got_full = download_benchmark_artifact(artifacts, 'all-benchmarks', branch, actual_commit_hash, overwrite, silent)

    return Status.OK if got_summary and got_full else Status.NO_BENCHMARK


def main():
    try:
        options = process_commandline()
        return download_benchmarks(
            options.branch,
            options.pull_request_id,
            options.base_of_pr,
            options.ignore_commit_hash,
            options.overwrite,
            options.debug_requests,
        ).value
    except JobNotSuccessful as exception:
        print(f"[ERROR] {exception}", file=sys.stderr)
        if not exception.job_finished:
            print("Please wait for the workflow to finish and try again.", file=sys.stderr)
            return Status.PENDING.value
        else:
            print("Benchmarks from this run of the pipeline are not available.", file=sys.stderr)
            return Status.NO_BENCHMARK.value
    except APIHelperError as exception:
        print(f"[ERROR] {exception}", file=sys.stderr)
        return Status.ERROR.value
    except requests.exceptions.HTTPError as exception:
        print(f"[ERROR] {exception}", file=sys.stderr)
        return Status.ERROR.value
    except RuntimeError as exception:
        print(f"[ERROR] {exception}", file=sys.stderr)
        return Status.ERROR.value


if __name__ == '__main__':
    sys.exit(main())
