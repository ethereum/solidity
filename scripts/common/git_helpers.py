import subprocess


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
