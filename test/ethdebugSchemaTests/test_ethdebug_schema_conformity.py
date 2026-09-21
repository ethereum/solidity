#!/usr/bin/env python3

import json
import re
import subprocess
import unittest
from functools import cache
from pathlib import Path

import jsonschema

# NOTE: test/ethdebugSchemaTests.py runs the tests with this directory in the import path.
# pragma pylint: disable=import-error
import schema_helpers
# pragma pylint: enable=import-error


TEST_DIR = Path(__file__).parent
# The isoltest cases in this directory pin down the resources of specific inputs;
# their output is checked against the schema here as well.
RESOURCES_TEST_SOURCES = sorted((TEST_DIR.parent / "libsolidity" / "ethdebugTests" / "resources").glob("*.sol"))
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
def resources_of_source(solc_path, source_path):
    process = subprocess.run(
        [solc_path, "--experimental", "--ethdebug-resources", str(source_path)],
        encoding="utf8",
        capture_output=True,
        check=True,
    )
    (resources, _) = json.JSONDecoder().raw_decode(process.stdout[process.stdout.index("{"):])
    return resources


def contract_outputs(solc_output):
    for (source_name, source_contracts) in solc_output["contracts"].items():
        for (contract_name, contract_output) in source_contracts.items():
            yield (source_name, contract_name, contract_output)


def referenced_type_ids(document):
    """The IDs of the type references `{"type": {"id": ...}}` inside a type document."""
    if isinstance(document, dict):
        for (key, value) in document.items():
            if key == "type" and isinstance(value, dict) and set(value) == {"id"}:
                yield value["id"]
            else:
                yield from referenced_type_ids(value)
    elif isinstance(document, list):
        for value in document:
            yield from referenced_type_ids(value)


# The compiler's type identifiers (Type::richIdentifier()) determine the document.
TYPE_ID_PATTERNS = {
    "uint": re.compile(r"^t_uint(\d+)$"),
    "int": re.compile(r"^t_int(\d+)$"),
    "bool": re.compile(r"^t_bool$"),
    "address": re.compile(r"^t_address(_payable)?$"),
    "fixed_bytes": re.compile(r"^t_bytes(\d+)$"),
    "bytes": re.compile(r"^t_bytes_(storage|memory|calldata)(_ptr)?$"),
    "string": re.compile(r"^t_string_(storage|memory|calldata)(_ptr)?$"),
    "fixed": re.compile(r"^t_(u?fixed)(\d+)x(\d+)$"),
    "array": re.compile(r"^t_array\$_(.+)_\$(dyn|\d+)_(storage|memory|calldata)(_ptr)?$"),
    "mapping": re.compile(r"^t_mapping\$_(.+?)_\$_(.+)_\$$"),
    "struct": re.compile(r"^t_struct\$_(\w+)_\$\d+_(storage|memory|calldata)(_ptr)?$"),
    "enum": re.compile(r"^t_enum\$_(\w+)_\$\d+$"),
    "contract": re.compile(r"^t_contract\$_(\w+)_\$\d+$"),
    "alias": re.compile(r"^t_userDefinedValueType\$_(\w+)_\$\d+$"),
    "function": re.compile(r"^t_function_(internal|external)_\w+\$_.*$"),
}


def classify_type_id(type_id):
    for (kind, pattern) in TYPE_ID_PATTERNS.items():
        match = pattern.match(type_id)
        if match:
            return (kind, match)
    return (None, None)


def pointer_expression_names(expression):
    """The variable and region names an expression refers to: (variables, regions)."""
    (variables, regions) = (set(), set())
    if isinstance(expression, str):
        if not expression.startswith("0x") and not expression.isdigit() and expression != "$wordsize":
            variables.add(expression)
    elif isinstance(expression, dict):
        for (key, operand) in expression.items():
            if key in (".slot", ".offset", ".length", "$read"):
                regions.add(operand)
            elif isinstance(operand, list):
                for element in operand:
                    (element_variables, element_regions) = pointer_expression_names(element)
                    variables |= element_variables
                    regions |= element_regions
            else:
                (operand_variables, operand_regions) = pointer_expression_names(operand)
                variables |= operand_variables
                regions |= operand_regions
    return (variables, regions)


def region_names(pointer):
    names = set()
    if isinstance(pointer, dict):
        if "location" in pointer and "name" in pointer:
            names.add(pointer["name"])
        for value in pointer.values():
            names |= region_names(value)
    elif isinstance(pointer, list):
        for value in pointer:
            names |= region_names(value)
    return names


@cache
def ethdebug_schema_repository():
    return schema_helpers.ethdebug_schema_repository(schema_helpers.ethdebug_schema_dir())


class EthdebugTestCase(unittest.TestCase):
    # Set by test/ethdebugSchemaTests.py.
    config = None

    @classmethod
    def setUpClass(cls):
        if cls.config is None:
            raise RuntimeError("No configuration. Run the tests through test/ethdebugSchemaTests.py.")

    def setUp(self):
        self.ethdebug_schema_repository = ethdebug_schema_repository()

    def validate(self, schema_id, instance):
        validator(schema_id, self.ethdebug_schema_repository).validate(instance)

    def assertPointerIsClosed(self, pointer, bound_variables, regions):
        """Every variable in the pointer is bound by a template parameter, a `define` or a
        `list`, and every region a lookup or `$read` refers to is named in the pointer."""

        def check_expression(expression):
            (variables, referenced_regions) = pointer_expression_names(expression)
            self.assertLessEqual(variables, bound_variables, f"Unbound variable in {expression}")
            self.assertLessEqual(referenced_regions, regions | {"$this"}, f"Unknown region in {expression}")

        if "location" in pointer:
            for key in ("slot", "offset", "length"):
                if key in pointer:
                    check_expression(pointer[key])
        elif "group" in pointer:
            for member in pointer["group"]:
                self.assertPointerIsClosed(member, bound_variables, regions)
        elif "list" in pointer:
            check_expression(pointer["list"]["count"])
            self.assertPointerIsClosed(pointer["list"]["is"], bound_variables | {pointer["list"]["each"]}, regions)
        elif "if" in pointer:
            check_expression(pointer["if"])
            self.assertPointerIsClosed(pointer["then"], bound_variables, regions)
            if "else" in pointer:
                self.assertPointerIsClosed(pointer["else"], bound_variables, regions)
        elif "define" in pointer:
            for expression in pointer["define"].values():
                check_expression(expression)
            self.assertPointerIsClosed(pointer["in"], bound_variables | set(pointer["define"]), regions)
        elif "templates" in pointer:
            self.assertPointerIsClosed(pointer["in"], bound_variables, regions)
        else:
            self.assertIn("template", pointer, f"Unknown pointer shape: {pointer}")

    def assertRegionCoversLayoutOffset(self, region, layout_offset):
        """A region's offset counts from the most significant byte of the slot, the storage
        layout's from the least significant one; a value of `length` bytes at layout offset
        `o` therefore starts at byte `32 - o - length`. A region without a length covers
        the rest of its slot, and one whose length exceeds a slot starts at the beginning
        of the first one."""
        if "length" not in region:
            self.assertNotIn("offset", region)
            self.assertEqual(layout_offset, 0)
            return
        offset = int(region.get("offset", "0x00"), 16)
        length = int(region["length"], 16)
        if length >= 32:
            self.assertEqual(offset, 0)
            self.assertEqual(layout_offset, 0)
        else:
            self.assertEqual(offset + length + layout_offset, 32)

    def assertTemplateIsClosed(self, template):
        self.assertEqual(len(template["expect"]), len(set(template["expect"])))
        self.assertPointerIsClosed(template["for"], set(template["expect"]), region_names(template["for"]))

    def assertTypeDocumentMatchesID(self, type_id, document, types):
        (kind, match) = classify_type_id(type_id)
        self.assertIsNotNone(kind, f"Type identifier of unknown form: {type_id}")
        if kind in ("uint", "int"):
            self.assertEqual(document, {"kind": kind, "bits": int(match.group(1))})
        elif kind == "bool":
            self.assertEqual(document, {"kind": "bool"})
        elif kind == "address":
            self.assertEqual(document, {"kind": "address", "payable": match.group(1) is not None})
        elif kind == "fixed_bytes":
            self.assertEqual(document, {"kind": "bytes", "size": int(match.group(1))})
        elif kind in ("bytes", "string"):
            self.assertEqual(document, {"kind": kind})
        elif kind == "fixed":
            self.assertEqual(document, {"kind": match.group(1), "bits": int(match.group(2)), "places": int(match.group(3))})
        elif kind == "array":
            self.assertEqual(document["kind"], "array")
            self.assertEqual(document["contains"], {"type": {"id": match.group(1)}})
            if match.group(2) == "dyn":
                self.assertNotIn("count", document)
            else:
                self.assertEqual(int(document["count"], 16), int(match.group(2)))
        elif kind == "mapping":
            self.assertEqual(document["kind"], "mapping")
            self.assertEqual(document["contains"]["key"], {"type": {"id": match.group(1)}})
            self.assertEqual(document["contains"]["value"], {"type": {"id": match.group(2)}})
        elif kind == "struct":
            self.assertEqual(document["kind"], "struct")
            self.assertEqual(document["definition"]["name"], match.group(1))
            for member in document["contains"]:
                self.assertIn("name", member)
        elif kind == "enum":
            self.assertEqual(document["kind"], "enum")
            self.assertEqual(document["definition"]["name"], match.group(1))
            self.assertGreater(len(document["values"]), 0)
        elif kind in ("contract", "alias"):
            self.assertEqual(document["kind"], kind)
            self.assertEqual(document["definition"]["name"], match.group(1))
        elif kind == "function":
            self.assertEqual(document["kind"], "function")
            self.assertIs(document.get(match.group(1)), True)
            self.assertEqual(document["contains"]["parameters"]["type"]["kind"], "tuple")
        for referenced_id in referenced_type_ids(document):
            self.assertIn(referenced_id, types, f"{type_id} references unknown type {referenced_id}")


class EthdebugSchemaConformityTest(EthdebugTestCase):

    @classmethod
    def setUpClass(cls):
        """
        Compiles the input once for the whole class, since every test reads the same output.
        """
        super().setUpClass()
        cls.standard_json_input = load_standard_json_input(Path(__file__).parent / "input_file.json")
        cls.solc_output = compile_standard_json(cls.config.solc_path, cls.standard_json_input)
        cls.resources = cls.solc_output["ethdebug"]["resources"]

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

    def test_type_documents_match_their_identifiers(self):
        types = self.resources["types"]
        self.assertGreater(len(types), 0)
        for (type_id, document) in types.items():
            with self.subTest(type=type_id):
                self.assertTypeDocumentMatchesID(type_id, document, types)

    def test_definition_locations_refer_to_the_compilation_sources(self):
        source_ids = {source["id"] for source in self.resources["compilation"]["sources"]}
        for (type_id, document) in self.resources["types"].items():
            location = document.get("definition", {}).get("location")
            if location is not None:
                with self.subTest(type=type_id):
                    self.assertIn(location["source"]["id"], source_ids)

    def test_pointer_templates_are_closed(self):
        for (name, template) in self.resources["pointers"].items():
            with self.subTest(pointer=name):
                self.assertTemplateIsClosed(template)

    def test_every_storage_variable_has_a_pointer_template(self):
        """The templates are named `<location>_<contract AST ID>_<variable AST ID>` and
        describe the variables the storage layouts list, at their slots."""
        contract_ids = {}
        for (source_name, source_output) in self.solc_output["sources"].items():
            for node in source_output["ast"]["nodes"]:
                if node["nodeType"] == "ContractDefinition":
                    contract_ids[f"{source_name}:{node['name']}"] = node["id"]

        layouts = {"storage": "storageLayout", "transient": "transientStorageLayout"}
        for (_, contract_name, contract_output) in contract_outputs(self.solc_output):
            for (location, layout_output) in layouts.items():
                for variable in contract_output[layout_output]["storage"]:
                    template_name = f"{location}_{contract_ids[variable['contract']]}_{variable['astId']}"
                    with self.subTest(contract=contract_name, variable=variable["label"]):
                        self.assertIn(template_name, self.resources["pointers"])
                        template = self.resources["pointers"][template_name]
                        # A mapping's keys are the template's parameters; everything else is closed.
                        self.assertEqual(len(template["expect"]) > 0, variable["type"].startswith("t_mapping"))
                        pointer = template["for"]
                        if "location" in pointer:
                            self.assertEqual(pointer["name"], variable["label"])
                            self.assertEqual(pointer["location"], location)
                            # A mapping value's slot is computed from the keys; anything else is at the layout's slot.
                            if not template["expect"]:
                                self.assertEqual(int(pointer["slot"], 16), int(variable["slot"]))
                                self.assertRegionCoversLayoutOffset(pointer, variable["offset"])
                        else:
                            self.assertTrue(
                                any(name.startswith(variable["label"]) for name in region_names(pointer)),
                                f"No region of {template_name} is named after {variable['label']}",
                            )


class ResourcesTestSourcesTest(EthdebugTestCase):
    """The resources of the isoltest cases under ethdebugTests/resources/."""

    def test_resources_conform_to_schema(self):
        assert RESOURCES_TEST_SOURCES, "No test sources found."
        for source_path in RESOURCES_TEST_SOURCES:
            with self.subTest(source=source_path.name):
                resources = resources_of_source(self.config.solc_path, source_path)
                self.validate("schema:ethdebug/format/info/resources", resources)
                self.assertGreater(len(resources["types"]), 0)

    def test_type_documents_match_their_identifiers(self):
        for source_path in RESOURCES_TEST_SOURCES:
            types = resources_of_source(self.config.solc_path, source_path)["types"]
            for (type_id, document) in types.items():
                with self.subTest(source=source_path.name, type=type_id):
                    self.assertTypeDocumentMatchesID(type_id, document, types)

    def test_pointer_templates_are_closed(self):
        for source_path in RESOURCES_TEST_SOURCES:
            for (name, template) in resources_of_source(self.config.solc_path, source_path)["pointers"].items():
                with self.subTest(source=source_path.name, pointer=name):
                    self.assertTemplateIsClosed(template)
