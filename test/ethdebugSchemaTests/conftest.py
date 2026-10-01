from pathlib import Path

import pytest

import schema_helpers


def pytest_addoption(parser):
    parser.addoption("--solc-binary-path", type=Path, required=True, help="Path to the solidity compiler binary.")


@pytest.fixture
def solc_path(request):
    solc_path = request.config.getoption("--solc-binary-path")
    assert solc_path.is_file()
    assert solc_path.exists()
    return solc_path


@pytest.fixture(scope="module")
def ethdebug_schema_dir():
    return schema_helpers.ethdebug_schema_dir()


@pytest.fixture(scope="module")
def ethdebug_schema_repository(ethdebug_schema_dir):
    return schema_helpers.ethdebug_schema_repository(ethdebug_schema_dir)
