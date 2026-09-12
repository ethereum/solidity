#!/usr/bin/env python

import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

# NOTE: This test file only works with scripts/ added to PYTHONPATH so pylint can't find the imports
# pragma pylint: disable=import-error
from error_codes import (
    REMOVED_IDS,
    check_removed_error_codes_between_branches,
    find_deleted_inventory_ids,
    fix_ids_in_source_file,
    find_ids_in_branch,
    find_inventory_ids_in_branch,
    find_newly_removed_ids,
    main,
)
# pragma pylint: enable=import-error


class TestErrorCodeBranchChecks(unittest.TestCase):
    def test_active_reused_ids_are_not_in_removed_inventory(self):
        active_reused_ids = {
            "2314", "2450", "2657", "3881", "5798", "5883", "6546", "9239",
        }
        self.assertEqual(active_reused_ids & REMOVED_IDS, set())

    def test_documentation_example_is_not_in_removed_inventory(self):
        self.assertNotIn("3141", REMOVED_IDS)

    def test_recently_removed_ids_are_in_inventory(self):
        self.assertTrue(
            {
                "1017", # Removed with experimental Solidity.
                "1305", # Removed with EOF support.
                "1481", # Removed from the storage layout checker.
            } <= REMOVED_IDS
        )

    def test_removing_id_from_inventory_is_reported(self):
        self.assertEqual(
            find_deleted_inventory_ids({"6635"}, set()),
            {"6635"},
        )

    @patch(
        "error_codes.subprocess.check_output",
        return_value='REMOVED_IDS = {"1234", "5678"}\n',
    )
    def test_inventory_is_loaded_from_base_commit(self, check_output):
        self.assertEqual(
            find_inventory_ids_in_branch("base-commit"),
            {"1234", "5678"},
        )
        check_output.assert_called_once_with(
            ["git", "show", "base-commit:scripts/error_codes.py"],
            universal_newlines=True,
        )

    def test_fix_replaces_removed_id(self):
        with tempfile.TemporaryDirectory() as tmp_dir:
            source_file = Path(tmp_dir) / "TypeChecker.cpp"
            source_file.write_text("reportError(6635_error);\n", encoding="utf-8")

            fix_ids_in_source_file(str(source_file), {"6635": 1}, {"9999"})

            self.assertEqual(source_file.read_text(encoding="utf-8"), "reportError(9999_error);\n")

    @patch("error_codes.examine_id_coverage", return_value=True)
    @patch(
        "error_codes.find_ids_in_source_files",
        return_value={"6635": ["/repo/liblangutil/ParserBase.cpp"]},
    )
    @patch("error_codes.find_source_files", return_value=["/repo/liblangutil/ParserBase.cpp"])
    @patch("error_codes.os.getcwd", return_value="/repo")
    def test_check_fails_for_removed_id_in_source(self, _cwd, _source_files, _ids, _coverage):
        with self.assertRaises(SystemExit) as exit_context:
            main(["--check"])

        self.assertEqual(exit_context.exception.code, 1)

    def test_newly_removed_id_without_inventory_is_reported(self):
        self.assertEqual(
            find_newly_removed_ids({"1234"}, set()), {"1234"}
        )

    def test_newly_removed_id_in_inventory_is_allowed(self):
        self.assertEqual(
            find_newly_removed_ids({"6635"}, set()), set()
        )

    @patch("error_codes.find_inventory_ids_in_branch", return_value=set())
    @patch("error_codes.find_ids_in_branch", return_value={"2314"})
    @patch("error_codes.subprocess.check_output", return_value="merge-base\n")
    def test_configured_comparison_target_is_used(self, check_output, find_ids, _inventory):
        self.assertTrue(check_removed_error_codes_between_branches({"2314"}, "pull-request-base"))

        check_output.assert_called_once_with(
            ["git", "merge-base", "pull-request-base", "HEAD"],
            universal_newlines=True,
        )
        find_ids.assert_called_once_with("merge-base")

    def test_missing_comparison_target_is_reported(self):
        with self.assertRaises(SystemExit) as exit_context:
            main(["--check-removed"])

        self.assertEqual(
            exit_context.exception.code,
            "Error: option --check-removed requires argument",
        )

    @patch("error_codes.find_inventory_ids_in_branch", return_value=set())
    @patch("error_codes.find_ids_in_branch", return_value=set())
    @patch("error_codes.subprocess.check_output", return_value="merge-base\n")
    def test_new_reuse_is_left_to_source_check(self, _check_output, _find_ids, _inventory):
        self.assertTrue(check_removed_error_codes_between_branches({"6635"}, "pull-request-base"))

    @patch("error_codes.find_inventory_ids_in_branch", return_value=set())
    @patch("error_codes.find_ids_in_branch", return_value={"1234"})
    @patch("error_codes.subprocess.check_output", return_value="merge-base\n")
    def test_uninventoried_removal_fails_the_branch_check(self, _check_output, _find_ids, _inventory):
        self.assertFalse(check_removed_error_codes_between_branches(set(), "pull-request-base"))

    @patch("error_codes.find_inventory_ids_in_branch", return_value={"9999"})
    @patch("error_codes.find_ids_in_branch", return_value=set())
    @patch("error_codes.subprocess.check_output", return_value="merge-base\n")
    def test_deleting_existing_inventory_entry_fails_branch_check(
        self,
        _check_output,
        _find_ids,
        _base_inventory,
    ):
        self.assertFalse(check_removed_error_codes_between_branches(set(), "pull-request-base"))

    @patch("error_codes.find_source_files", side_effect=RuntimeError("scan failed"))
    @patch("error_codes.subprocess.run")
    def test_temporary_worktree_is_removed_if_scanning_fails(self, run, _find_source_files):
        with self.assertRaisesRegex(RuntimeError, "scan failed"):
            find_ids_in_branch("base-commit")

        self.assertEqual(run.call_count, 2)
        self.assertEqual(run.call_args_list[1].args[0][0:3], ["git", "worktree", "remove"])
