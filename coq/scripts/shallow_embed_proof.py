import json
from pathlib import Path
import sys

def indent(level):
    return "  " * level

def node_in_block_to_coq(definition_name: str, level: int, node) -> str:
    node_type = node.get('nodeType')

    if node_type in ['YulVariableDeclaration', 'YulAssignment']:
        return node_to_coq(definition_name, level, node)

    elif node_type in ['YulBlock', 'YulIf', 'YulSwitch']:
        return \
            "(* block *)\n" + \
            indent(level) + "eapply Compare.Let. {\n" + \
            indent(level + 1) + "Compare.Tactic.stack_primitives.\n" + \
            indent(level + 1) + node_to_coq(definition_name, level + 1, node) + "\n" + \
            indent(level) + "}\n" + \
            indent(level) + "Compare.Tactic.make_intro."

    return \
        "eapply Compare.Let. {\n" + \
        indent(level + 1) + node_to_coq(definition_name, level + 1, node) + "\n" + \
        indent(level) + "}\n" + \
        indent(level) + "Compare.Tactic.make_intro."

def block_to_coq(definition_name: str, level: int, node) -> str:
    node_type = node.get('nodeType')

    if node_type == 'YulBlock':
        statements = [
            node_in_block_to_coq(definition_name, level, stmt)
            for stmt in node.get('statements', [])
            if stmt.get('nodeType') != 'YulFunctionDefinition'
        ]
        return \
            ("\n" + indent(level)).join(statements)

    return "(* Unsupported block node type: {node_type} *)"

def expression_to_coq(definition_name: str) -> str:
    return f"Compare.Tactic.expression ltac:({definition_name}_deps)."

def node_to_coq(definition_name: str, level: int, node) -> str:
    if isinstance(node, dict):
        node_type = node.get('nodeType')

        if node_type == 'YulBlock':
            return block_to_coq(definition_name, level, node)

        elif node_type == 'YulFunctionDefinition':
            return "(* Function definition should be handled at top level *)"

        elif node_type in ['YulVariableDeclaration', 'YulAssignment']:
            return \
                "(* declaration/assignment *)\n" + \
                indent(level) + "apply Compare.LetUnfold.\n" + \
                indent(level) + "Compare.Tactic.stack_primitives.\n" + \
                indent(level) + expression_to_coq(definition_name)

        elif node_type == 'YulFunctionCall':
            return "(* Unexpected function call *)"

        elif node_type == 'YulIdentifier':
            return node.get('name', 'Unknown identifier')

        elif node_type == 'YulLiteral':
            if node['kind'] == 'string':
                return "0x" + node['hexValue']
            return node.get('value', 'Unknown literal')

        elif node_type == 'YulExpressionStatement':
            return \
                "(* expression statement *)\n" + \
                indent(level) + expression_to_coq(definition_name)

        elif node_type == 'YulIf':
            true_body = node_to_coq(definition_name, level, node.get('body'))
            return \
                "(* if *)\n" + \
                indent(level) + expression_to_coq(definition_name) + "\n" + \
                indent(level) + "Compare.Tactic.open_if.\n" + \
                indent(level) + "Compare.Tactic.stack_primitives.\n" + \
                indent(level) + true_body

        elif node_type == 'YulSwitch':
            cases = [
                "(* case *)\n" + \
                indent(level) + "Compare.Tactic.open_switch_case. {\n" + \
                indent(level + 1) + "Compare.Tactic.stack_primitives.\n" + \
                indent(level + 1) + node_to_coq(definition_name, level + 1, case.get('body')) + "\n" + \
                indent(level) + "}"
                for case in node.get('cases', [])
            ]
            return \
                "(* switch *)\n" + \
                indent(level) + expression_to_coq(definition_name) + "\n" + \
                indent(level) + ("\n" + indent(level)).join(cases) + "\n" + \
                indent(level) + "now apply Compare.Pure."

        else:
            return f"(* Unsupported node type: {node_type} *)"

    else:
        # A node should always be a dictionary
        return f"(* Unsupported node: {node} *)"

def deps_tactic(definition_name: str, local_functions: list[str], level: int) -> str:
    return \
        f"Ltac {definition_name}_deps :=\n" + \
        indent(level + 1) + (
            "idtac" \
            if len(local_functions) == 0 \
            else (" ||\n" + indent(level + 1)).join("apply compare_" + local_function for local_function in local_functions)
        ) + "."

def get_code(contract_name: str, is_deployed_code: bool) -> str:
    return contract_name.capitalize() + (".deployed" if is_deployed_code else "") + ".code"

def function_definition_to_coq(
    contract_name: str,
    module_prefix: list[str],
    is_deployed_code: bool,
    local_functions: list[str],
    level: int,
    node
) -> str:
    definition_name = node.get('name')
    params = [
        p['name']
        for p in node.get('parameters', [])
    ]
    lemma_params = \
        "" \
        if len(params) == 0 \
        else " (" + " ".join(params) + " : U256.t)"
    code = get_code(contract_name, is_deployed_code)
    body = block_to_coq(definition_name, level + 1, node.get('body'))
    return \
        deps_tactic(definition_name, local_functions, level) + "\n" + \
        "\n" + \
        indent(level) + f"Lemma compare_{definition_name} environment stack{lemma_params} :\n" + \
        indent(level + 1) + "let environment :=\n" + \
        indent(level + 2) + f"environment <| Environment.code_name := {code}.(Code.hex_name) |> in\n" + \
        indent(level + 1) + "let function :=\n" + \
        indent(level + 2) + f"Codes.get_function {contract_name}.codes environment \"" + definition_name + "\" in\n" + \
        indent(level + 1) + f"Compare.t {contract_name}.codes environment stack stack\n" + \
        indent(level + 2) + "(function [" + "; ".join(params) + "])\n" + \
        indent(level + 2) + "(" + ".".join([contract_name + "_shallow"] + module_prefix + [definition_name]) + "".join(" "+ param for param in params) + ").\n" + \
        indent(level) + "Proof.\n" + \
        indent(level + 1) + "(* entering function *)\n" + \
        indent(level + 1) + "Compare.Tactic.stack_primitives.\n" + \
        indent(level + 1) + body + "\n" + \
        indent(level) + "Qed."

def block_definition_to_coq(
    contract_name: str,
    module_prefix: list[str],
    is_deployed_code: bool,
    local_functions: list[str],
    level: int,
    node
) -> str:
    code = get_code(contract_name, is_deployed_code)
    body = block_to_coq("body", level + 1, node)
    return \
        deps_tactic("body", local_functions, level) + "\n" + \
        "\n" + \
        indent(level) + "Lemma compare_body environment stack :\n" + \
        indent(level + 1) + "let environment :=\n" + \
        indent(level + 2) + f"environment <| Environment.code_name := {code}.(Code.hex_name) |> in\n" + \
        indent(level + 1) + f"Compare.t {contract_name}.codes environment stack stack\n" + \
        indent(level + 2) + f"{code}.(Code.body)\n" + \
        indent(level + 2) + ".".join([contract_name + "_shallow"] + module_prefix + ["body"]) + ".\n" + \
        indent(level) + "Proof.\n" + \
        indent(level + 1) + "(* entering function *)\n" + \
        indent(level + 1) + "Compare.Tactic.stack_primitives.\n" + \
        indent(level + 1) + body + "\n" + \
        indent(level) + "Qed."

def top_level_to_coq(
    contract_name: str,
    module_prefix: list[str],
    is_deployed_code: bool,
    level,
    node
) -> str:
    node_type = node.get('nodeType')

    if node_type == 'YulBlock':
        local_functions = []
        lemmas = []
        for stmt in node.get('statements', []):
            if stmt.get('nodeType') == 'YulFunctionDefinition':
                lemma = function_definition_to_coq(contract_name, module_prefix, is_deployed_code, local_functions, level, stmt)
                lemmas.append(lemma)
                local_functions.append(stmt.get('name'))
        body_lemma = block_definition_to_coq(contract_name, module_prefix, is_deployed_code, local_functions, level, node)
        lemmas.append(body_lemma)

        return ("\n\n" + indent(level)).join(lemmas)

    return f"(* Unsupported top-level node type: {node_type} *)"

def object_to_coq(
    contract_name: str,
    module_prefix: list[str],
    is_deployed_code: bool,
    level: int,
    node
) -> str:
    node_type = node.get('nodeType')

    if node_type == 'YulObject':
        name = node['name']
        module_prefix = module_prefix + [name]
        return \
            "Module " + name + ".\n" + \
            indent(level + 1) + object_to_coq(contract_name, module_prefix, is_deployed_code, level + 1, node['code']) + "\n" + \
            "".join(
                "\n" +
                indent(level + 1) + object_to_coq(contract_name, module_prefix, True, level + 1, child) + "\n"
                for child in node.get('subObjects', [])
                if child.get('nodeType') != 'YulData'
            ) + \
            indent(level) + "End " + node['name'] + "."

    elif node_type == 'YulCode':
        return top_level_to_coq(contract_name, module_prefix, is_deployed_code, level, node['block'])

    elif node_type == 'YulData':
        return "(* Data object not expected *)"

    return f"(* Unsupported object node type: {node_type} *)"

def main():
    """Input: JSON file with Yul AST"""
    json_input = sys.argv[1]
    with open(json_input, 'r') as file:
        data = json.load(file)

    contract_name = Path(json_input).stem
    import_path = str(Path(json_input).with_suffix('')).replace('/', '.')
    coq_code = object_to_coq(contract_name, [], False, 0, data)

    print("(* Generated by " + Path(__file__).name + " *)")
    print("Require Import CoqOfSolidity.CoqOfSolidity.")
    print("Require Import CoqOfSolidity.simulations.CoqOfSolidity.")
    print(f"""Require Import {import_path}.
Require Import {import_path}_shallow.

Import Run.""")
    print()
    print(coq_code)

if __name__ == "__main__":
    main()
