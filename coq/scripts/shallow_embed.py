from collections import defaultdict
from dataclasses import dataclass
import json
from pathlib import Path
import sys
from typing import Literal, Union


# Indent each line of the block, except empty lines
def indent(block: str) -> str:
    indentation = "  "
    return "\n".join(
        line if line == "" else indentation + line
        for line in block.split("\n")
    )


def paren(condition: bool, value: str) -> str:
    return f"({value})" if condition else value


def variable_name_to_coq(name: str) -> str:
    reserved_names = [
        "end",
        "return",
    ]

    if name in reserved_names:
        return name + "_"

    return name.replace("$", "'dollar'")


def variables_names_to_coq(as_pattern: bool, variable_names: list) -> str:
    if len(variable_names) == 1:
        return variable_name_to_coq(variable_names[0].get('name'))
    else:
        quote = "'" if as_pattern else ""
        return \
            quote + "(" + \
            ', '.join(
                variable_name_to_coq(variable_name.get('name'))
                for variable_name in variable_names
            ) + \
            ")"


@dataclass(frozen=True)
class Effect:
    new_vars: set[str]
    updated_vars: set[str]


empty_effect: Effect = Effect(set(), set())


def node_in_block_to_coq(local_vars: set[str], node) -> tuple[str, Effect]:
    node_type = node.get('nodeType')

    if node_type in ['YulVariableDeclaration', 'YulAssignment']:
        return node_to_coq(local_vars, node)

    if node_type in ['YulIf', 'YulSwitch']:
        coq, effect = node_to_coq(local_vars, node)
        return (
            "do~ [[\n" +
            indent(coq) + "\n" +
            "]] in",
            effect,
        )

    if node_type in ['YulBlock', 'YulForLoop']:
        return (
            "do~\n" +
            indent(node_to_coq(local_vars, node)[0]) + "\n" +
            "in",
            empty_effect,
        )

    if node_type in ['YulExpressionStatement']:
        return (
            "do~ [[ " + node_to_coq(local_vars, node)[0] + " ]] in",
            empty_effect,
        )

    return (
        "(* Unsupported node type: {node_type} *)",
        empty_effect,
    )


def block_to_coq(local_vars: set[str], node, result: str) -> tuple[str, Effect]:
    node_type = node.get('nodeType')

    if node_type == 'YulBlock':
        current_local_vars = local_vars
        current_updated_vars: set[str] = set()
        statements = []
        for stmt in node.get('statements', []):
            if stmt.get('nodeType') == 'YulFunctionDefinition':
                continue
            statement, effect = node_in_block_to_coq(current_local_vars, stmt)
            statements += [statement]
            current_local_vars |= effect.new_vars
            current_updated_vars |= effect.updated_vars
        statements += [result]
        state_variables = paren(
            True,
            ", ".join(variable_name_to_coq(name) for name in current_updated_vars)
        )
        return (
            f"fun '{state_variables} mode =>\n" +
            "\n".join(statements),
            Effect(
                new_vars=set(),
                updated_vars=current_updated_vars,
            ),
        )

    return (
        "(* Unsupported block node type: {node_type} *)",
        empty_effect,
    )


def is_function_pure(function_name: str) -> bool:
    # For now we do not do specific analysis to detect the pure functions
    pure_functions: list[str] = [
        "add",
        "sub",
        "mul",
        "div",
        "sdiv",
        "mod_",
        "smod",
        "exp",
        "not",
        "lt",
        "gt",
        "slt",
        "sgt",
        "eq",
        "iszero",
        "and",
        "or",
        "xor",
        "byte",
        "shl",
        "shr",
        "sar",
        "addmod",
        "mulmod",
        "signextend",
    ]
    # return function_name in pure_functions
    return False


def get_effect(
    local_vars: set[str],
    bound_variable_names: list,
    mode: Union[Literal["declaration"], Literal["assignment"]],
) -> Effect:
    new_vars = {
        variable.get('name')
        for variable in bound_variable_names
        if variable.get('name') not in local_vars
    }
    updated_vars = {
        variable.get('name')
        for variable in bound_variable_names
        if variable.get('name') in local_vars
    }

    if mode == "declaration" and len(updated_vars) > 0:
        print(
            f"Warning: Declaration: {updated_vars} already in scope {local_vars}",
            file=sys.stderr
        )
    if mode == "assignment" and len(new_vars) > 0:
        print(
            f"Warning: Assignment: {new_vars} not in scope {local_vars}",
            file=sys.stderr
        )

    return Effect(new_vars, updated_vars)


def node_to_coq(local_vars: set[str], node) -> tuple[str, Effect]:
    if isinstance(node, dict):
        node_type = node.get('nodeType')

        if node_type == 'YulBlock':
            return block_to_coq(local_vars, node, "M.pure tt")

        elif node_type == 'YulFunctionDefinition':
            return (
                "(* Function definition is only handled at top level *)",
                empty_effect
            )

        elif node_type == 'YulVariableDeclaration':
            variable_names = node.get('variables', [])
            variables = variables_names_to_coq(True, variable_names)
            value, _ = node_to_coq(local_vars, node.get('value'))
            return (
                f"let~ {variables} := [[ {value} ]] in",
                get_effect(local_vars, variable_names, "declaration"),
            )

        elif node_type == 'YulAssignment':
            variable_names = node.get('variableNames', [])
            variables = variables_names_to_coq(True, variable_names)
            value, _ = node_to_coq(local_vars, node.get('value'))
            return (
                f"let~ {variables} := [[ {value} ]] in",
                get_effect(local_vars, variable_names, "assignment"),
            )

        elif node_type == 'YulFunctionCall':
            func_name = variable_name_to_coq(node['functionName']['name'])
            is_pure = is_function_pure(func_name)
            if is_pure:
                func_name = "Pure." + func_name
            args: list[str] = [
                paren(
                    arg.get('nodeType') not in ['YulLiteral', 'YulIdentifier'],
                    node_to_coq(local_vars, arg)[0],
                )
                for arg in node.get('arguments', [])
            ]
            if is_pure:
                return (
                    func_name + "".join(" " + arg for arg in args),
                    empty_effect,
                )
            args_left = "~(|"
            args_right = "|)"
            return (
                func_name + " " + \
                (args_left + args_right \
                if len(node.get('arguments', [])) == 0 \
                else \
                    args_left + " " + \
                    ', '.join(args) + \
                    " " + args_right),
                empty_effect,
            )

        elif node_type == 'YulIdentifier':
            name = node.get('name', 'Unknown identifier')
            # Sanity check for the environment of variables
            if name not in local_vars:
                print(f"Warning: Expression: variable {name} not in scope {local_vars}")
            return (
                variable_name_to_coq(name),
                empty_effect,
            )

        elif node_type == 'YulLiteral':
            if node['kind'] == 'string':
                return (
                    "0x" + node['hexValue'].ljust(64, '0') +
                    " (* " + node['value'] + " *)",
                    empty_effect,
                )
            return (
                node.get('value', 'Unknown literal'),
                empty_effect,
            )

        elif node_type == 'YulExpressionStatement':
            return node_to_coq(local_vars, node.get('expression'))

        elif node_type == 'YulIf':
            condition, _ = node_to_coq(local_vars, node.get('condition'))
            true_body, true_effect = node_to_coq(local_vars, node.get('body'))
            return (
                f"M.if_unit (| {condition},\n" +
                indent(true_body) + "\n" +
                "|)",
                Effect(
                    new_vars=set(),
                    updated_vars=true_effect.updated_vars,
                ),
            )

        elif node_type == 'YulSwitch':
            expression, _ = node_to_coq(local_vars, node.get('expression'))
            cases = [
                f"if δ =? {node_to_coq(local_vars, case.get('value'))[0]} then\n" +
                indent(node_to_coq(local_vars, case.get('body'))[0])
                for case in node.get('cases', [])
            ]
            return (
                "(* switch *)\n" +
                f"let* δ := ltac:(M.monadic ({expression})) in\n" +
                ("\nelse ").join(cases) + "\n" +
                "else\n" +
                indent("M.pure tt"),
                empty_effect,
            )

        elif node_type == 'YulLeave':
            return ("M.leave", empty_effect)

        elif node_type == 'YulBreak':
            return ("M.break", empty_effect)

        elif node_type == 'YulContinue':
            return ("M.continue", empty_effect)

        elif node_type == 'YulForLoop':
            pre, _ = node_in_block_to_coq(local_vars, node.get('pre'))
            condition, _ = node_to_coq(local_vars, node.get('condition'))
            post, _ = node_to_coq(local_vars, node.get('post'))
            body, _ = node_to_coq(local_vars, node.get('body'))

            return (
                "(* for loop *)\n" +
                "(* pre *)\n" +
                pre + "\n" +
                "M.for_unit\n" +
                indent(
                    "(* condition *)\n" +
                    "[[ " + condition + " ]]\n" +
                    "(* body *)\n" +
                    "(" + body + ")\n" +
                    "(* post *)\n" +
                    "(" + post + ")"
                ),
                empty_effect,
            )

        else:
            return (f"(* Unsupported node type: {node_type} *)", empty_effect)

    else:
        # A node should always be a dictionary
        return (f"(* Unsupported node: {node} *)", empty_effect)


def function_result_value(returnVariables: list) -> str:
    if len(returnVariables) == 0:
        return "M.pure tt"

    return "M.pure " + variables_names_to_coq(False, returnVariables)


def function_result_type(arity: int) -> str:
    if arity == 0:
        return "unit"
    elif arity == 1:
        return "U256.t"

    return "(" + " * ".join(["U256.t"] * arity) + ")"


def function_definition_to_coq(node) -> str:
    name = variable_name_to_coq(node.get('name'))
    param_names: list[str] = [
        param['name']
        for param in node.get('parameters', [])
    ]
    params = ''.join([
        " (" + variable_name_to_coq(name) + " : U256.t)"
        for name in param_names
    ])
    result_names: list[str] = [
        result['name']
        for result in node.get('returnVariables', [])
    ]
    result = function_result_value(node.get('returnVariables', []))
    body, _ = block_to_coq(
        set(param_names) | set(result_names),
        node.get('body'),
        result,
    )
    return \
        f"Definition {name}{params} : M.t " + \
        function_result_type(len(node.get('returnVariables', []))) + " :=\n" + \
        indent(body) + "."


# Get the names of the functions called in a function.
# We take care of sorting the names in alphabetical order so that the output is
# deterministic.
def get_function_dependencies(function_node) -> list[str]:
    dependencies = set()

    def traverse(node):
        if isinstance(node, dict):
            if node.get('nodeType') == 'YulFunctionCall':
                function_name = node['functionName']['name']
                dependencies.add(function_name)
            for key in sorted(node.keys()):
                traverse(node[key])
        elif isinstance(node, list):
            for item in node:
                traverse(item)

    # Start traversal from the 'statements' field
    traverse(function_node.get('body', {}))

    return sorted(dependencies)


def topological_sort(functions: dict[str, list[str]]) -> list[str]:
    # Create a graph representation
    graph = defaultdict(list)
    all_funcs = set()
    for func, called_funcs in sorted(functions.items()):
        all_funcs.add(func)
        for called in called_funcs:
            graph[func].append(called)
            all_funcs.add(called)

    # Helper function for DFS
    def dfs(node, visited, stack, path):
        visited.add(node)
        path.add(node)

        for neighbor in graph[node]:
            if neighbor in path:
                cycle = list(path)[list(path).index(neighbor):] + [neighbor]
                print(f"Warning: Cycle detected: {' -> '.join(cycle)}")
            elif neighbor not in visited:
                dfs(neighbor, visited, stack, path)

        path.remove(node)
        stack.append(node)

    visited = set()
    stack = []

    # Perform DFS for each unvisited node
    for func in sorted(all_funcs):
        if func not in visited:
            dfs(func, visited, stack, set())

    return stack


def order_functions(ordered_names: list[str], function_nodes: list) -> list:
    # Create a dictionary for quick lookup of index in ordered_names
    name_order: dict[str, int] = {
        name: index
        for index, name in enumerate(ordered_names)
    }

    # Define a key function that returns the index of the function name in ordered_names
    def key_func(node):
        return name_order.get(node.get('name'), len(ordered_names))

    # Sort the function_nodes using the key function
    return sorted(function_nodes, key=key_func)


def top_level_to_coq(node) -> str:
    node_type = node.get('nodeType')

    if node_type == 'YulBlock':
        functions_dependencies: dict[str, list[str]] = {}
        for statement in node.get('statements', []):
            if statement.get('nodeType') == 'YulFunctionDefinition':
                function_name = statement.get('name')
                dependencies = get_function_dependencies(statement)
                functions_dependencies[function_name] = dependencies
        ordered_function_names = topological_sort(functions_dependencies)
        ordered_functions = \
            order_functions(ordered_function_names, node.get('statements', []))
        functions = [
            function_definition_to_coq(function)
            for function in ordered_functions
            if function.get('nodeType') == 'YulFunctionDefinition'
        ]
        body = \
            "Definition body : M.t unit :=\n" + \
            indent(node_to_coq(set(), node)[0]) + "."
        return ("\n\n").join(functions + [body])

    return f"(* Unsupported top-level node type: {node_type} *)"


def object_to_coq(node) -> str:
    node_type = node.get('nodeType')

    if node_type == 'YulObject':
        return \
            "Module " + node['name'] + ".\n" + \
            indent(object_to_coq(node['code'])) + "\n" + \
            "".join(
                "\n" +
                indent(object_to_coq(child)) + "\n"
                for child in node.get('subObjects', [])
                if child.get('nodeType') != 'YulData'
            ) + \
            "End " + node['name'] + "."

    elif node_type == 'YulCode':
        return top_level_to_coq(node['block'])

    elif node_type == 'YulData':
        return "(* Data object not expected *)"

    return f"(* Unsupported object node type: {node_type} *)"


def main():
    """Input: JSON file with Yul AST"""
    with open(sys.argv[1], 'r') as file:
        data = json.load(file)

    coq_code = object_to_coq(data)

    print("(* Generated by " + Path(__file__).name + " *)")
    print("Require Import CoqOfSolidity.CoqOfSolidity.")
    print("Require Import CoqOfSolidity.simulations.CoqOfSolidity.")
    print("Import Stdlib.")
    print()
    print(coq_code)


if __name__ == "__main__":
    main()
