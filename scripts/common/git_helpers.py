import subprocess
from pathlib import Path
from shutil import which


def run_git_command(command):
    process = subprocess.run(
        command,
        encoding='utf8',
        capture_output=True,
        check=True,
    )
    return process.stdout.strip()


def git_current_branch():
    return run_git_command(['git', 'symbolic-ref', 'HEAD', '--short'])


def git_commit_hash(ref: str = 'HEAD'):
    return run_git_command(['git', 'rev-parse', '--verify', ref])


def git_diff(file_a: Path, file_b: Path) -> int:
    if which('git') is None:
        raise RuntimeError('git not found.')

    return subprocess.run([
        'git',
        'diff',
        '--color',
        '--word-diff=plain',
        '--word-diff-regex=.',
        '--ignore-space-change',
        '--ignore-blank-lines',
        '--exit-code',
        file_a,
        file_b,
    ], encoding='utf8', check=False).returncode
