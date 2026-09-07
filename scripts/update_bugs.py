#!/usr/bin/env python3

import json
from pathlib import Path

import yaml


ROOT_PATH = Path(__file__).resolve().parent.parent
BUGS_YAML = ROOT_PATH / "docs" / "bugs.yaml"
BUGS_JSON = ROOT_PATH / "docs" / "bugs.json"


def update_bugs(input_path=BUGS_YAML, output_path=BUGS_JSON):
    bugs = yaml.safe_load(input_path.read_text(encoding="utf8"))

    output_path.write_text(
        json.dumps(
            bugs,
            sort_keys=False,
            indent=4,
            separators=(",", ": "),
        ),
        encoding="utf8",
    )


if __name__ == "__main__":
    update_bugs()