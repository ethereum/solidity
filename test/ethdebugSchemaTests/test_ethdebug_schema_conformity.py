#!/usr/bin/env python3

import json
import subprocess
import unittest
from pathlib import Path

import jsonschema

# NOTE: test/ethdebugSchemaTests.py runs the tests with this directory in the import path.
# pragma pylint: disable=import-error
import schema_helpers
# pragma pylint: enable=import-error


def get_nested_value(dictionary, *keys):
    for key in keys:
        dictionary = dictionary[key]
    return dictionary


def validator(schema_id, ethdebug_schema_repository):
    return jsonschema.Draft202012Validator(
        schema={"$ref": schema_id},
        registry=ethdebug_schema_repository
    )


def ethdebug_programs(solc_output, output_selection):
    assert "contracts" in solc_output
    for source_name, source_contracts in solc_output["contracts"].items():
        assert len(source_contracts) > 0
        for contract_name, contract_output in source_contracts.items():
            yield source_name, contract_name, get_nested_value(contract_output, *(output_selection.split(".")))


def load_standard_json_input(path):
    with open(path, "r", encoding="utf8") as f:
        standard_json_input = json.load(f)

    for source in standard_json_input["sources"].values():
        if "contentFile" in source:
            source["content"] = (path.parent / source.pop("contentFile")).read_text(encoding="utf8")

    return standard_json_input


def compile_standard_json(solc_path, standard_json_input):
    process = subprocess.run(
        [solc_path, "--standard-json"],
        input=json.dumps(standard_json_input),
        encoding="utf8",
        capture_output=True,
        check=True,
    )
    assert process.returncode == 0
    return json.loads(process.stdout)


class EthdebugSchemaConformityTest(unittest.TestCase):
    # Set by test/ethdebugSchemaTests.py.
    config = None

    def setUp(self):
        assert self.config is not None, "Run the tests through test/ethdebugSchemaTests.py."
        self.standard_json_input = load_standard_json_input(Path(__file__).parent / "input_file.json")
        self.solc_output = compile_standard_json(self.config.solc_path, self.standard_json_input)
        self.ethdebug_schema_repository = schema_helpers.ethdebug_schema_repository(
            schema_helpers.ethdebug_schema_dir()
        )

    def test_program_schema(self):
        program_validator = validator("schema:ethdebug/format/program", self.ethdebug_schema_repository)
        for output_selection in ("evm.bytecode.ethdebug", "evm.deployedBytecode.ethdebug"):
            for (_, contract_name, ethdebug_data) in ethdebug_programs(self.solc_output, output_selection):
                with self.subTest(output_selection=output_selection, contract=contract_name):
                    program_validator.validate(ethdebug_data)

    def test_resources_schema(self):
        resources_validator = validator("schema:ethdebug/format/info/resources", self.ethdebug_schema_repository)
        resources_validator.validate(self.solc_output["ethdebug"]["resources"])

    def test_compilation_schema(self):
        compilation_validator = validator("schema:ethdebug/format/materials/compilation", self.ethdebug_schema_repository)
        compilation_validator.validate(self.solc_output["ethdebug"]["compilation"])

    def test_program_sanity(self):
        source_ids = {source_name: source["id"] for (source_name, source) in self.solc_output["sources"].items()}

        for (output_selection, environment) in (
            ("evm.bytecode.ethdebug", "create"),
            ("evm.deployedBytecode.ethdebug", "call"),
        ):
            for (source_name, contract_name, ethdebug_data) in ethdebug_programs(self.solc_output, output_selection):
                with self.subTest(output_selection=output_selection, contract=contract_name):
                    self.assertEqual(ethdebug_data["environment"], environment)
                    self.assertEqual(ethdebug_data["contract"]["name"], contract_name)
                    self.assertEqual(ethdebug_data["contract"]["definition"]["source"]["id"], source_ids[source_name])

                    instructions = ethdebug_data["instructions"]
                    self.assertGreater(len(instructions), 0)
                    self.assertEqual(
                        [instruction["offset"] for instruction in instructions],
                        sorted(instruction["offset"] for instruction in instructions),
                    )
                    self.assertTrue(all(instruction["operation"]["mnemonic"] for instruction in instructions))

    def test_resources_match_standard_json_sources(self):
        standard_json_sources = {
            source_name: source["id"] for (source_name, source) in self.solc_output["sources"].items()
        }
        ethdebug_sources = {
            source["path"]: source["id"]
            for source in self.solc_output["ethdebug"]["resources"]["compilation"]["sources"]
        }
        self.assertEqual(ethdebug_sources, standard_json_sources)

    def test_resources_include_standard_json_source_contents(self):
        ethdebug_sources = {
            source["path"]: source
            for source in self.solc_output["ethdebug"]["resources"]["compilation"]["sources"]
        }

        self.assertEqual(set(ethdebug_sources), set(self.standard_json_input["sources"]))
        for (source_name, source_input) in self.standard_json_input["sources"].items():
            self.assertEqual(ethdebug_sources[source_name]["contents"], source_input["content"])
            self.assertEqual(ethdebug_sources[source_name]["language"], "Solidity")

    def test_resources_include_empty_type_and_pointer_tables(self):
        self.assertEqual(self.solc_output["ethdebug"]["resources"]["types"], {})
        self.assertEqual(self.solc_output["ethdebug"]["resources"]["pointers"], {})

    def test_resources_and_compilation_share_compilation(self):
        self.assertEqual(self.solc_output["ethdebug"]["resources"]["compilation"], self.solc_output["ethdebug"]["compilation"])
