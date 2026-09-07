#!/usr/bin/env python3

import json
import tempfile
import unittest
from pathlib import Path

import update_bugs


class UpdateBugsTest(unittest.TestCase):
    def test_generates_equivalent_json(self):
        bugs_yaml = Path(__file__).parent.parent.parent / "docs" / "bugs.yaml"
        expected_json = Path(__file__).parent.parent.parent / "docs" / "bugs.json"

        with tempfile.TemporaryDirectory() as temp_dir:
            output_json = Path(temp_dir) / "bugs.json"

            update_bugs.update_bugs(bugs_yaml, output_json)

            generated = json.loads(output_json.read_text(encoding="utf8"))
            expected = json.loads(expected_json.read_text(encoding="utf8"))

            self.assertEqual(generated, expected)


if __name__ == "__main__":
    unittest.main()