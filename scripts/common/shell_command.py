#!/usr/bin/env python3
import os
import subprocess

def run_cmd(command: str, env: dict = None, logfile: str = None) -> int:
    """
    Args:
        command: command to run
        logfile: log file name
        env:     dictionary holding key-value pairs for bash environment variables
    Returns:
        int: The exit status of the command. Exit status codes are:
            0     -> Success
            1-255 -> Failure
    """
    if logfile is None:
        logfile = os.devnull
    if env is None:
        env = os.environ.copy()
    with open(
        file=logfile,
        mode='w',
        encoding='utf8'
    ) as log:
        ret = subprocess.run(
            command,
            shell=True,
            check=True,
            executable='/bin/bash',
            env=env,
            stdout=log if not logfile else None,
            stderr=None
        )
        return ret.returncode
