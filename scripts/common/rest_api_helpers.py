from pathlib import Path
from typing import List, Mapping, Optional
import functools
import json
import operator
import shutil

import requests


class APIHelperError(Exception):
    pass

class JobNotSuccessful(APIHelperError):
    def __init__(self, name: str, status: str):
        assert status != 'success'

        self.name = name
        self.status = status
        self.job_finished = (status in ['failed', 'blocked'])

        if status == 'not_running':
            message = f"Job {name} has not started yet."
        elif status == 'blocked':
            message = f"Job {name} will not run because one of its dependencies failed."
        elif status == 'running':
            message = f"Job {name} is still running."
        elif status == 'failed':
            message = f"Job {name} failed."
        else:
            message = f"Job {name} did not finish successfully. Current status: {status}."

        super().__init__(message)

class JobMissing(APIHelperError):
    pass

class InvalidResponse(APIHelperError):
    pass

class FileAlreadyExists(APIHelperError):
    pass


def query_api(url: str, params: Mapping[str, str], debug_requests=False) -> dict:
    if debug_requests:
        print(f'REQUEST URL: {url}')
        if len(params) > 0:
            print(f'QUERY: {params}')

    response = requests.get(url, params=params, timeout=60)
    response.raise_for_status()

    if debug_requests:
        json_response = response.json()
        print('========== RESPONSE ==========')
        if json_response is not None:
            print(json.dumps(json_response, indent=4))
        else:
            print(response.content)
        print('==============================')

    return response.json()


def download_file(url: str, target_path: Path, overwrite=False):
    if not overwrite and target_path.exists():
        raise FileAlreadyExists(f"Refusing to overwrite existing file: '{target_path}'.")

    with requests.get(url, stream=True, timeout=60) as request:
        with open(target_path, 'wb') as target_file:
            shutil.copyfileobj(request.raw, target_file)


class Github:
    BASE_URL = 'https://api.github.com'

    project_slug: str
    debug_requests: bool

    def __init__(self, project_slug: str, debug_requests: bool):
        self.project_slug = project_slug
        self.debug_requests = debug_requests

    def pull_request(self, pr_id: int) -> dict:
        return query_api(
            f'{self.BASE_URL}/repos/{self.project_slug}/pulls/{pr_id}',
            {},
            self.debug_requests
        )


class CircleCI:
    # None might be a more logical default for max_pages but in most cases we'll actually
    # want some limit to prevent flooding the API with requests in case of a bug.
    DEFAULT_MAX_PAGES = 10
    BASE_URL = 'https://circleci.com/api/v2'

    project_slug: str
    debug_requests: bool

    def __init__(self, project_slug: str, debug_requests: bool):
        self.project_slug = project_slug
        self.debug_requests = debug_requests

    def paginated_query_api_iterator(self, url: str, params: Mapping[str, str], max_pages: int = DEFAULT_MAX_PAGES):
        assert 'page-token' not in params

        page_count = 0
        next_page_token = None
        while max_pages is None or page_count < max_pages:
            if next_page_token is not None:
                params = {**params, 'page-token': next_page_token}

            json_response = query_api(url, params, self.debug_requests)

            yield json_response['items']
            next_page_token = json_response['next_page_token']
            page_count += 1
            if next_page_token is None:
                break

    def paginated_query_api(self, url: str, params: Mapping[str, str], max_pages: int = DEFAULT_MAX_PAGES):
        return functools.reduce(operator.add, self.paginated_query_api_iterator(url, params, max_pages), [])

    def pipelines(
        self,
        branch: Optional[str] = None,
        commit_hash: Optional[str] = None,
        excluded_trigger_types: List[str] = None,
    ) -> List[dict]:
        if excluded_trigger_types is None:
            excluded_trigger_types = []

        for items in self.paginated_query_api_iterator(
            f'{self.BASE_URL}/project/gh/{self.project_slug}/pipeline',
            {'branch': branch} if branch is not None else {},
            max_pages=10,
        ):
            matching_items = [
                item
                for item in items
                if (
                    (commit_hash is None or item['vcs']['revision'] == commit_hash) and
                    item['trigger']['type'] not in excluded_trigger_types
                )
            ]
            if len(matching_items) > 0:
                return matching_items

        return []

    def workflows(self, pipeline_id: str) -> dict:
        return self.paginated_query_api(f'{self.BASE_URL}/pipeline/{pipeline_id}/workflow', {})

    def jobs(self, workflow_id: str) -> Mapping[str, dict]:
        items = self.paginated_query_api(f'{self.BASE_URL}/workflow/{workflow_id}/job', {})
        jobs_by_name = {job['name']: job for job in items}

        assert len(jobs_by_name) <= len(items)
        if len(jobs_by_name) < len(items):
            raise InvalidResponse("Job names in the workflow are not unique.")

        return jobs_by_name

    def job(self, workflow_id: str, name: str, require_success: bool = False) -> dict:
        jobs = self.jobs(workflow_id)
        if name not in jobs:
            raise JobMissing(f"Job {name} is not present in the workflow.")

        if require_success and jobs[name]['status'] != 'success':
            raise JobNotSuccessful(name, jobs[name]['status'])

        return jobs[name]

    def artifacts(self, job_number: int) -> Mapping[str, dict]:
        items = self.paginated_query_api(f'{self.BASE_URL}/project/gh/{self.project_slug}/{job_number}/artifacts', {})
        artifacts_by_name = {artifact['path']: artifact for artifact in items}

        assert len(artifacts_by_name) <= len(items)
        if len(artifacts_by_name) < len(items):
            raise InvalidResponse("Names of artifacts attached to the job are not unique.")

        return artifacts_by_name

    @staticmethod
    def latest_item(items: dict) -> dict:
        sorted_items = sorted(items, key=lambda item: item['created_at'], reverse=True)
        return sorted_items[0] if len(sorted_items) > 0 else None
