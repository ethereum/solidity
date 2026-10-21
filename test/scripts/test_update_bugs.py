#!/usr/bin/env python3

"""Tests for the known-bugs YAML to JSON generator."""

import json
import tempfile
import unittest
from pathlib import Path

import yaml

import update_bugs_json

DOCS_DIR = Path(__file__).parent.parent.parent / "docs"

VALID_BUG_YAML = """\
- uid: SOL-2026-1
  name: SomeBug
  summary: >-
    Summary.
  description: >-
    Description.
  link: https://blog.soliditylang.org/
  introduced: 0.8.20
  fixed: 0.8.30
  publish: "2026-01-01"
  severity: low/medium
  conditions:
    viaIR: true
    evmVersion: ">=cancun"
  check:
    regex-source: "foo"
"""


class UpdateBugsTest(unittest.TestCase):
    """Test generation of the JSON bug list from YAML."""

    def test_generates_equivalent_json(self):
        """Verify that generated JSON preserves the existing bug data."""
        bugs_yaml = DOCS_DIR / "bugs.yaml"
        expected_json = DOCS_DIR / "bugs.json"

        with tempfile.TemporaryDirectory() as temp_dir:
            output_json = Path(temp_dir) / "bugs.json"

            update_bugs_json.update_bugs(bugs_yaml, output_json)

            generated = json.loads(output_json.read_text(encoding="utf8"))
            expected = json.loads(expected_json.read_text(encoding="utf8"))

            self.assertEqual(generated, expected)


class ValidateBugsTest(unittest.TestCase):
    """Test validation of the YAML bug list against the schema."""

    @staticmethod
    def validate(bugs_yaml):
        update_bugs_json.validate_bugs(yaml.safe_load(bugs_yaml))

    def test_bug_list_conforms_to_schema(self):
        update_bugs_json.validate_bugs(json.loads((DOCS_DIR / "bugs.json").read_text(encoding="utf8")))

    def test_accepts_all_fields(self):
        self.validate(VALID_BUG_YAML)

    def test_rejects_non_array_list(self):
        with self.assertRaises(update_bugs_json.BugListValidationError) as context:
            self.validate("uid: SOL-2026-1\n")
        self.assertIn("$: {'uid': 'SOL-2026-1'} is not of type 'array'", str(context.exception))

    def test_rejects_non_object_entry(self):
        with self.assertRaises(update_bugs_json.BugListValidationError) as context:
            self.validate(VALID_BUG_YAML + "- SOL-2026-2\n")
        self.assertIn("entry #1: <entry>: 'SOL-2026-2' is not of type 'object'", str(context.exception))

    def test_reports_entry_index_when_uid_missing(self):
        with self.assertRaises(update_bugs_json.BugListValidationError) as context:
            self.validate(VALID_BUG_YAML.replace("- uid: SOL-2026-1\n  name:", "- name:"))
        self.assertIn("entry #0: <entry>: 'uid' is a required property", str(context.exception))

    def test_reports_entry_index_when_uid_empty(self):
        with self.assertRaises(update_bugs_json.BugListValidationError) as context:
            self.validate(VALID_BUG_YAML.replace("uid: SOL-2026-1", "uid:"))
        self.assertIn("entry #0: uid: None is not of type 'string'", str(context.exception))

    def test_rejects_version_parsed_as_number(self):
        with self.assertRaises(update_bugs_json.BugListValidationError) as context:
            self.validate(VALID_BUG_YAML.replace("fixed: 0.8.30", "fixed: 0.9"))
        self.assertIn("SOL-2026-1: fixed: 0.9 is not of type 'string'", str(context.exception))

    def test_rejects_unquoted_date(self):
        with self.assertRaises(update_bugs_json.BugListValidationError) as context:
            self.validate(VALID_BUG_YAML.replace('publish: "2026-01-01"', "publish: 2026-01-01"))
        self.assertIn("SOL-2026-1: publish: datetime.date(2026, 1, 1) is not of type 'string'", str(context.exception))

    def test_rejects_invalid_date(self):
        with self.assertRaises(update_bugs_json.BugListValidationError) as context:
            self.validate(VALID_BUG_YAML.replace('publish: "2026-01-01"', 'publish: "2026-13-01"'))
        self.assertIn("SOL-2026-1: publish: '2026-13-01' is not a 'date'", str(context.exception))

    def test_rejects_unknown_key(self):
        with self.assertRaises(update_bugs_json.BugListValidationError) as context:
            self.validate(VALID_BUG_YAML + "  fixedIn: 0.8.30\n")
        self.assertIn(
            "SOL-2026-1: <entry>: Additional properties are not allowed ('fixedIn' was unexpected)",
            str(context.exception)
        )

    def test_rejects_missing_key(self):
        with self.assertRaises(update_bugs_json.BugListValidationError) as context:
            self.validate(VALID_BUG_YAML.replace("  severity: low/medium\n", ""))
        self.assertIn("SOL-2026-1: <entry>: 'severity' is a required property", str(context.exception))

    def test_rejects_unknown_severity(self):
        with self.assertRaises(update_bugs_json.BugListValidationError) as context:
            self.validate(VALID_BUG_YAML.replace("severity: low/medium", "severity: critical"))
        self.assertIn("SOL-2026-1: severity: 'critical' is not one of", str(context.exception))

    def test_rejects_unknown_condition(self):
        with self.assertRaises(update_bugs_json.BugListValidationError) as context:
            self.validate(VALID_BUG_YAML.replace("viaIR: true", "viaIr: true"))
        self.assertIn(
            "SOL-2026-1: conditions: Additional properties are not allowed ('viaIr' was unexpected)",
            str(context.exception)
        )

    def test_rejects_non_boolean_condition(self):
        with self.assertRaises(update_bugs_json.BugListValidationError) as context:
            self.validate(VALID_BUG_YAML.replace("viaIR: true", "viaIR: yes please"))
        self.assertIn("SOL-2026-1: conditions.viaIR: 'yes please' is not of type 'boolean'", str(context.exception))

    def test_rejects_unknown_check(self):
        with self.assertRaises(update_bugs_json.BugListValidationError) as context:
            self.validate(VALID_BUG_YAML.replace("regex-source:", "source-regex:"))
        self.assertIn(
            "SOL-2026-1: check: Additional properties are not allowed ('source-regex' was unexpected)",
            str(context.exception)
        )

    def test_rejects_duplicate_name(self):
        with self.assertRaises(update_bugs_json.BugListValidationError) as context:
            self.validate(VALID_BUG_YAML + "\n" + VALID_BUG_YAML.replace("SOL-2026-1", "SOL-2026-2"))
        self.assertIn("Duplicate bug name: SomeBug", str(context.exception))


if __name__ == "__main__":
    unittest.main()
