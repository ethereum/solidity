#!/usr/bin/env python3

import json
import subprocess
from pathlib import Path

import jsonschema
import pytest


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
            # Interfaces and libraries without bytecode have no program output.
            try:
                program = get_nested_value(contract_output, *(output_selection.split(".")))
            except KeyError:
                continue
            if program is None:
                continue
            yield source_name, contract_name, program


def load_standard_json_input(path):
    with open(path, "r", encoding="utf8") as f:
        standard_json_input = json.load(f)

    for source in standard_json_input["sources"].values():
        if "contentFile" in source:
            source["content"] = (path.parent / source.pop("contentFile")).read_text(encoding="utf8")

    return standard_json_input


@pytest.fixture(params=["input_file.json"])
def standard_json_input(request):
    testfile_dir = Path(__file__).parent
    return load_standard_json_input(testfile_dir / request.param)


@pytest.fixture
def solc_output(standard_json_input, solc_path):
    process = subprocess.run(
        [solc_path, "--standard-json"],
        input=json.dumps(standard_json_input),
        encoding="utf8",
        capture_output=True,
        check=True,
    )
    assert process.returncode == 0
    return json.loads(process.stdout)


@pytest.mark.parametrize("output_selection", ["evm.bytecode.ethdebug", "evm.deployedBytecode.ethdebug"], ids=str)
def test_program_schema(
    output_selection,
    ethdebug_schema_repository,
    solc_output
):
    program_validator = validator("schema:ethdebug/format/program", ethdebug_schema_repository)
    for _, _, ethdebug_data in ethdebug_programs(solc_output, output_selection):
        program_validator.validate(ethdebug_data)


def test_resources_schema(ethdebug_schema_repository, solc_output):
    resources_validator = validator("schema:ethdebug/format/info/resources", ethdebug_schema_repository)
    resources_validator.validate(solc_output["ethdebug"]["resources"])


def test_compilation_schema(ethdebug_schema_repository, solc_output):
    compilation_validator = validator("schema:ethdebug/format/materials/compilation", ethdebug_schema_repository)
    compilation_validator.validate(solc_output["ethdebug"]["compilation"])


@pytest.mark.parametrize(
    ("output_selection", "environment"),
    [
        ("evm.bytecode.ethdebug", "create"),
        ("evm.deployedBytecode.ethdebug", "call"),
    ],
    ids=str
)
def test_program_sanity(output_selection, environment, solc_output):
    source_ids = {source_name: source["id"] for source_name, source in solc_output["sources"].items()}

    for source_name, contract_name, ethdebug_data in ethdebug_programs(solc_output, output_selection):
        assert ethdebug_data["environment"] == environment
        assert ethdebug_data["contract"]["name"] == contract_name
        assert ethdebug_data["contract"]["definition"]["source"]["id"] == source_ids[source_name]

        instructions = ethdebug_data["instructions"]
        assert len(instructions) > 0
        assert [instruction["offset"] for instruction in instructions] == sorted(
            instruction["offset"] for instruction in instructions
        )
        assert all(instruction["operation"]["mnemonic"] for instruction in instructions)


def test_resources_match_standard_json_sources(solc_output):
    standard_json_sources = {source_name: source["id"] for source_name, source in solc_output["sources"].items()}
    ethdebug_sources = {
        source["path"]: source["id"]
        for source in solc_output["ethdebug"]["resources"]["compilation"]["sources"]
    }
    assert ethdebug_sources == standard_json_sources


def test_resources_include_standard_json_source_contents(standard_json_input, solc_output):
    ethdebug_sources = {
        source["path"]: source
        for source in solc_output["ethdebug"]["resources"]["compilation"]["sources"]
    }

    assert set(ethdebug_sources) == set(standard_json_input["sources"])
    for source_name, source_input in standard_json_input["sources"].items():
        assert ethdebug_sources[source_name]["contents"] == source_input["content"]
        assert ethdebug_sources[source_name]["language"] == "Solidity"


def referenced_type_ids(document):
    """The IDs of the type references `{"type": {"id": ...}}` inside a type document."""
    if isinstance(document, dict):
        for key, value in document.items():
            if key == "type" and isinstance(value, dict) and set(value) == {"id"}:
                yield value["id"]
            else:
                yield from referenced_type_ids(value)
    elif isinstance(document, list):
        for value in document:
            yield from referenced_type_ids(value)


def test_resources_type_table_is_closed(solc_output):
    types = solc_output["ethdebug"]["resources"]["types"]
    assert len(types) > 0
    # Composed types reference their component types by ID into this table.
    for type_id, document in types.items():
        for referenced_id in referenced_type_ids(document):
            assert referenced_id in types, f"{type_id} references unknown type {referenced_id}"


def test_resources_include_every_type_kind(solc_output):
    types = solc_output["ethdebug"]["resources"]["types"]
    kinds = {document["kind"] for document in types.values()}
    # Tuples only occur inline, as the parameter lists of function types.
    assert kinds >= {
        "uint", "int", "bool", "bytes", "string", "address", "contract", "enum",
        "alias", "array", "mapping", "struct", "function",
    }

    assert types["t_uint128"] == {"kind": "uint", "bits": 128}
    assert types["t_int256"] == {"kind": "int", "bits": 256}
    assert types["t_bool"] == {"kind": "bool"}
    assert types["t_bytes32"] == {"kind": "bytes", "size": 32}
    assert types["t_bytes_storage"] == {"kind": "bytes"}
    assert types["t_string_storage"] == {"kind": "string"}
    assert types["t_address"] == {"kind": "address", "payable": False}
    assert types["t_address_payable"] == {"kind": "address", "payable": True}
    assert types["t_array$_t_uint16_$8_storage"] == {
        "kind": "array",
        "contains": {"type": {"id": "t_uint16"}},
        "count": "0x08",
    }
    assert types["t_array$_t_uint256_$dyn_storage"] == {
        "kind": "array",
        "contains": {"type": {"id": "t_uint256"}},
    }
    assert types["t_mapping$_t_address_$_t_uint256_$"] == {
        "kind": "mapping",
        "contains": {
            "key": {"type": {"id": "t_address"}},
            "value": {"type": {"id": "t_uint256"}},
        },
    }

    enums = [document for document in types.values() if document["kind"] == "enum"]
    assert [document["values"] for document in enums] == [["Red", "Green", "Blue"]]
    assert enums[0]["definition"]["name"] == "Color"

    structs = {document["definition"]["name"]: document for document in types.values() if document["kind"] == "struct"}
    assert [member["name"] for member in structs["Point"]["contains"]] == ["x", "y", "salt"]
    assert structs["Point"]["contains"][2]["type"] == {"id": "t_bytes4"}
    assert [member["name"] for member in structs["Line"]["contains"]] == ["from", "to", "label"]

    aliases = [document for document in types.values() if document["kind"] == "alias"]
    assert [document["definition"]["name"] for document in aliases] == ["Price"]
    assert aliases[0]["contains"] == {"type": {"id": "t_uint128"}}

    contracts = {document["definition"]["name"]: document for document in types.values() if document["kind"] == "contract"}
    assert contracts["I"]["interface"] is True and "library" not in contracts["I"]
    assert "interface" not in contracts["A1"] and "library" not in contracts["A1"]

    functions = {
        "internal" if document.get("internal") else "external": document
        for document in types.values() if document["kind"] == "function"
    }
    assert functions["internal"]["contains"]["parameters"]["type"] == {
        "kind": "tuple",
        "contains": [{"type": {"id": "t_uint256"}}],
    }
    assert functions["external"]["contains"]["returns"]["type"]["contains"] == [{"type": {"id": "t_bool"}}]


def test_resources_include_state_variable_pointer_templates(solc_output):
    pointers = solc_output["ethdebug"]["resources"]["pointers"]
    by_name = {}
    for template in pointers.values():
        target = template["for"]
        names = [target["name"]] if "name" in target else [member.get("name") for member in target.get("group", [])]
        for name in names:
            if name:
                by_name.setdefault(name, template)

    # Value types are single regions; packed members carry offset and length.
    assert by_name["stored"] == {
        "expect": [],
        "for": {"name": "stored", "location": "storage", "slot": "0x00", "length": "0x10"},
    }
    assert by_name["enabled"] == {
        "expect": [],
        "for": {"name": "enabled", "location": "storage", "slot": "0x00", "offset": "0x10", "length": "0x01"},
    }
    # Transient storage variables are addressed the same way in their own location.
    assert by_name["temporary"]["for"] == {"name": "temporary", "location": "transient", "slot": "0x00"}
    # Constants have no storage and therefore no pointer.
    assert "CONSTANT" not in by_name

    # Mapping pointers are templates over their expected keys; nested mappings expect one key per level.
    balances = [template for template in pointers.values() if template["for"].get("name") == "balances"][0]
    assert balances == {
        "expect": ["key"],
        "for": {
            "name": "balances",
            "location": "storage",
            "slot": {"$keccak256": [{"$wordsized": "key"}, {"$wordsized": "0x0f"}]},
        },
    }
    lines = [template for template in pointers.values() if template["expect"] == ["key", "key1"]]
    assert len(lines) == 1 and lines[0]["for"]["group"][0]["group"][0]["name"] == "lines-from-x"

    def template_whose_first_region_is(name):
        return [
            template for template in pointers.values()
            if template["for"].get("group", [{}])[0].get("name") == name
        ][0]

    # Dynamic arrays: the length in the base slot, the data at keccak256(slot).
    values_pointer = template_whose_first_region_is("values-length")["for"]
    assert values_pointer["group"][0]["slot"] == "0x0d"
    assert values_pointer["group"][1]["define"] == {"values-data": {"$keccak256": [{"$wordsized": "0x0d"}]}}
    values_list = values_pointer["group"][1]["in"]["list"]
    assert values_list["count"] == {"$read": "values-length"}
    assert values_list["each"] == "values-index"
    assert values_list["is"]["slot"] == {"$sum": ["values-data", "values-index"]}

    # Strings and bytes use the compact encoding, selected by the length flag.
    label_pointer = template_whose_first_region_is("label-length-flag")["for"]
    assert label_pointer["group"][0]["offset"] == {"$difference": ["$wordsize", "0x01"]}
    conditional = label_pointer["group"][1]
    assert "if" in conditional and "then" in conditional and "else" in conditional


def test_resources_and_compilation_share_compilation(solc_output):
    assert solc_output["ethdebug"]["resources"]["compilation"] == solc_output["ethdebug"]["compilation"]


ETHDEBUG_GOLDEN_TESTS = sorted((Path(__file__).parent.parent / "libsolidity" / "ethdebugTests").glob("resources_*.sol"))


@pytest.mark.parametrize("source_path", ETHDEBUG_GOLDEN_TESTS, ids=lambda path: path.name)
def test_resources_of_golden_tests_conform_to_schema(solc_path, ethdebug_schema_repository, source_path):
    """The resources emitted for the golden tests' inputs are schema-compliant, whatever they contain."""
    process = subprocess.run(
        [solc_path, "--experimental", "--ethdebug-resources", str(source_path)],
        encoding="utf8",
        capture_output=True,
        check=True,
    )
    resources, _ = json.JSONDecoder().raw_decode(process.stdout[process.stdout.index("{"):])
    validator("schema:ethdebug/format/info/resources", ethdebug_schema_repository).validate(resources)
    assert len(resources["types"]) > 0
