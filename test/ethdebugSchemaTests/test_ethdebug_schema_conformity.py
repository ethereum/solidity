#!/usr/bin/env python3

import json
import subprocess
import unittest
from functools import cache
from pathlib import Path

import jsonschema

# NOTE: test/ethdebugSchemaTests.py runs the tests with this directory in the import path.
# pragma pylint: disable=import-error
import schema_helpers
# pragma pylint: enable=import-error


PROGRAM_OUTPUTS = {"evm.bytecode.ethdebug": "create", "evm.deployedBytecode.ethdebug": "call"}


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
    for (source_name, source_contracts) in solc_output["contracts"].items():
        assert len(source_contracts) > 0
        for (contract_name, contract_output) in source_contracts.items():
            # Interfaces and abstract contracts have no bytecode and therefore no program.
            try:
                program = get_nested_value(contract_output, *(output_selection.split(".")))
            except KeyError:
                continue
            if program is not None:
                yield (source_name, contract_name, program)


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


@cache
def ethdebug_schema_repository():
    return schema_helpers.ethdebug_schema_repository(schema_helpers.ethdebug_schema_dir())


class EthdebugSchemaConformityTest(unittest.TestCase):

    # Set by test/ethdebugSchemaTests.py.
    config = None

    @classmethod
    def setUpClass(cls):
        """
        Compiles the input once for the whole class, since every test reads the same output.
        """
        if cls.config is None:
            raise RuntimeError("No configuration. Run the tests through test/ethdebugSchemaTests.py.")
        cls.standard_json_input = load_standard_json_input(Path(__file__).parent / "input_file.json")
        cls.solc_output = compile_standard_json(cls.config.solc_path, cls.standard_json_input)

    def setUp(self):
        self.ethdebug_schema_repository = ethdebug_schema_repository()

    def check_program_schema(self, output_selection):
        program_validator = validator("schema:ethdebug/format/program", self.ethdebug_schema_repository)
        for (_, contract_name, ethdebug_data) in ethdebug_programs(self.solc_output, output_selection):
            with self.subTest(contract=contract_name):
                program_validator.validate(ethdebug_data)

    def test_creation_program_schema(self):
        self.check_program_schema("evm.bytecode.ethdebug")

    def test_deployed_program_schema(self):
        self.check_program_schema("evm.deployedBytecode.ethdebug")

    def test_resources_schema(self):
        resources_validator = validator("schema:ethdebug/format/info/resources", self.ethdebug_schema_repository)
        resources_validator.validate(self.solc_output["ethdebug"]["resources"])

    def test_compilation_schema(self):
        compilation_validator = validator("schema:ethdebug/format/materials/compilation", self.ethdebug_schema_repository)
        compilation_validator.validate(self.solc_output["ethdebug"]["compilation"])

    def check_program_sanity(self, output_selection, environment):
        source_ids = {source_name: source["id"] for (source_name, source) in self.solc_output["sources"].items()}

        for (source_name, contract_name, ethdebug_data) in ethdebug_programs(self.solc_output, output_selection):
            with self.subTest(contract=contract_name):
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

    def test_creation_program_sanity(self):
        self.check_program_sanity("evm.bytecode.ethdebug", "create")

    def test_deployed_program_sanity(self):
        self.check_program_sanity("evm.deployedBytecode.ethdebug", "call")

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

    def test_resources_and_compilation_share_compilation(self):
        self.assertEqual(self.solc_output["ethdebug"]["resources"]["compilation"], self.solc_output["ethdebug"]["compilation"])
