from pathlib import Path
from typing import Union

LIBSOLIDITY_TEST_DIR = Path(__file__).parent.parent / 'libsolidity'
FIXTURE_DIR = Path(__file__).parent / 'fixtures'

def load_file(path: Union[Path, str]) -> str:
    with open(path, 'r', encoding='utf-8') as f:
        return f.read()

def load_fixture(relative_path: Union[Path, str]) -> str:
    return load_file(FIXTURE_DIR / relative_path)

def load_libsolidity_test_case(relative_path: Union[Path, str]) -> str:
    return load_file(LIBSOLIDITY_TEST_DIR / relative_path)
