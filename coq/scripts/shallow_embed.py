from collections import defaultdict
import json
from pathlib import Path
import sys
from typing import Callable, Literal, Union


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

    quote = "'" if as_pattern else ""
    return \
        quote + "(" + \
        ', '.join(
            variable_name_to_coq(variable_name.get('name'))
            for variable_name in variable_names
        ) + \
        ")"


def updated_vars_to_tuple(as_pattern: bool, updated_vars: set[str]) -> str:
    sorted_vars = sorted(updated_vars)

    if len(sorted_vars) == 0:
        return "'tt" if as_pattern else "tt"

    if len(sorted_vars) == 1:
        return variable_name_to_coq(sorted_vars[0])

    quote = "'" if as_pattern else ""
    return \
        quote + "(" + \
        ', '.join(
            variable_name_to_coq(var)
            for var in sorted_vars
        ) + \
        ")"


def block_write(updated_vars: set[str]) -> str:
    return "M.pure (BlockUnit.Tt, " + updated_vars_to_tuple(False, updated_vars) + ")"


def block_to_coq(
    updated_vars: set[str],
    mode: Union[Literal["once"], Literal["repeat"]],
    node,
) -> tuple[Callable[[str], str], set[str]]:
    node_type = node.get('nodeType')

    if node_type == 'YulBlock':
        statements, updated_vars = statements_to_coq(
            updated_vars,
            node.get('statements', []),
        )
        return (
            lambda result: "\n".join(statements + [result]),
            updated_vars,
        )

    return (
        lambda _: "(* Unsupported block node type: {node_type} *)",
        set(),
    )


ReturnMode = Literal[
    "tt",
    "break",
    "continue",
    "leave",
]


# We authorize the definition of local functions as lambdas, otherwise this makes
# another error.
# ruff: noqa: E731
def statements_to_coq(
    updated_vars: set[str],
    nodes: list,
) -> tuple[list[str], ReturnMode, set[str]]:
    return_mode: ReturnMode = "tt"

    if len(nodes) > 0:
        last_node = nodes[-1]
        if isinstance(last_node, dict):
            last_node_type = last_node.get('nodeType')
            if last_node_type == 'YulBreak':
                return_mode = "break"
                nodes = nodes[:-1]
            elif last_node_type == 'YulContinue':
                return_mode = "continue"
                nodes = nodes[:-1]
            elif last_node_type == 'YulLeave':
                return_mode = "leave"
                nodes = nodes[:-1]

    if len(nodes) == 0:
        return ([], return_mode, updated_vars)

    [node, *rest] = nodes
    get_statement: Callable[[set[str]], str]
    shadowed_vars: set[str] = set()

    if isinstance(node, dict):
        node_type = node.get('nodeType')

        if node_type == 'YulBlock':
            statement, statement_updated_vars = block_to_coq(
                set(),
                "once",
                node,
            )
            get_statement = lambda _: \
                "do~\n" + \
                indent(statement(block_write(updated_vars))) + "\n" + \
                "in"
            updated_vars |= statement_updated_vars

        elif node_type == 'YulFunctionDefinition':
            # We ignore this case because we only handle top-level function definitions
            return statements_to_coq(updated_vars, rest)

        elif node_type == 'YulVariableDeclaration':
            variable_names = node.get('variables', [])
            variables = variables_names_to_coq(True, variable_names)
            value = expression_to_coq(node.get('value'))
            get_statement = lambda _: \
                f"let~ {variables} := [[ {value} ]] in"
            shadowed_vars = {
                variable_name.get('name')
                for variable_name in variable_names
            }

        elif node_type == 'YulAssignment':
            variable_names = node.get('variableNames', [])
            variables = variables_names_to_coq(True, variable_names)
            value = expression_to_coq(node.get('value'))
            get_statement = lambda _: \
                f"let~ {variables} := [[ {value} ]] in"
            updated_vars |= {
                variable_name.get('name')
                for variable_name in variable_names
            }

        elif node_type == 'YulExpressionStatement':
            get_statement = lambda _: \
                "do~ [[ " + expression_to_coq(node.get('expression')) + " ]] in"

        elif node_type == 'YulIf':
            condition = expression_to_coq(node.get('condition'))
            then_body, then_updated_vars = block_to_coq(
                set(),
                'once',
                node.get('body'),
            )
            get_statement = lambda _: \
                "let~ (* state *) '(_, " + \
                updated_vars_to_tuple(False, then_updated_vars) + \
                ") := [[\n" + \
                indent(
                    "Shallow.if_ (|\n" +
                    indent(updated_vars_to_tuple(False, then_updated_vars)) + ",\n" +
                    indent(condition) + ",\n" +
                    indent(then_body(block_write(then_updated_vars))) + "\n" +
                    "|)"
                ) + "\n" + \
                "]] in"
            updated_vars |= then_updated_vars

        elif node_type == 'YulSwitch':
            expression = expression_to_coq(node.get('expression'))
            cases = [
                (
                    expression_to_coq(case.get('value')),
                    block_to_coq(
                        set(),
                        'once',
                        case.get('body'),
                    ),
                )
                for case in node.get('cases', [])
            ]
            commonly_updated_vars: set[str] = {
                name
                for _, (_, updated_vars) in cases
                for name in updated_vars
            }
            get_statement = lambda _: \
                "let~ (* state *) " + updated_vars_to_tuple(True, commonly_updated_vars) + \
                " := [[\n" + \
                indent(
                    "(* switch *)\n" + \
                    f"let* δ := [[ {expression} ]] in\n" + \
                    "\nelse ".join(
                        f"if δ =? {value} then\n" +
                        indent(body(block_write(commonly_updated_vars))) + "\n"
                        for value, (body, _) in cases
                    ) + "\n" + \
                    "else\n" + \
                    indent("M.pure tt")
                ) + "\n" + \
                "]] in"
            updated_vars |= commonly_updated_vars

        elif node_type == 'YulLeave':
            get_statement = lambda _: "M.leave"

        elif node_type == 'YulBreak':
            get_statement = lambda _: "M.break"

        elif node_type == 'YulContinue':
            get_statement = lambda _: "M.continue"

        elif node_type == 'YulForLoop':
            # We have not yet seen a case where the pre block is not empty.
            pre, pre_updated_vars = block_to_coq(
                set(),
                'once',
                node.get('pre'),
            )
            condition = expression_to_coq(node.get('condition'))
            post, post_updated_vars = block_to_coq(
                set(),
                'repeat',
                node.get('post'),
            )
            body, body_updated_vars = block_to_coq(
                set(),
                'repeat',
                node.get('body'),
            )
            commonly_updated_vars = \
                pre_updated_vars | post_updated_vars | body_updated_vars
            get_statement = lambda _: \
                "do~\n" + \
                indent(
                    "(* for loop *)\n" + \
                    "(* pre *)\n" + \
                    pre(block_write(pre_updated_vars)) + "\n" + \
                    "M.for_unit\n" + \
                    indent(
                        "(* condition *)\n" + \
                        "[[ " + condition + " ]]\n" + \
                        "(* body *)\n" + \
                        "(" + body(block_write(commonly_updated_vars)) + ")\n" + \
                        "(* post *)\n" + \
                        "(" + post(block_write(commonly_updated_vars)) + ")"
                    )
                ) + "\n" + \
                "in"
            updated_vars |= pre_updated_vars | post_updated_vars | body_updated_vars

        else:
            get_statement = lambda _: \
                f"(* Unsupported statement node type: {node_type} *)"

    else:
        # A node should always be a dictionary
        get_statement = lambda _: "(* Expected a dictionary node *)"

    statements, updated_vars = statements_to_coq(
        updated_vars,
        rest,
    )
    return (
        [get_statement(updated_vars)] + statements,
        return_mode,
        updated_vars - shadowed_vars,
    )


def expression_to_coq(node) -> str:
    node_type = node.get('nodeType')

    if node_type == 'YulFunctionCall':
        func_name = variable_name_to_coq(node['functionName']['name'])
        args: list[str] = [
            paren(
                arg.get('nodeType') == 'YulFunctionCall',
                expression_to_coq(arg),
            )
            for arg in node.get('arguments', [])
        ]
        args_left = "~(|"
        args_right = "|)"
        return func_name + " " + \
            (args_left + args_right \
            if len(node.get('arguments', [])) == 0 \
            else \
                args_left + " " + \
                ', '.join(args) + \
                " " + args_right)

    if node_type == 'YulIdentifier':
        return variable_name_to_coq(node.get('name'))

    if node_type == 'YulLiteral':
        if node['kind'] == 'string':
            return \
                "0x" + node['hexValue'].ljust(64, '0') + \
                " (* " + node['value'] + " *)"
        return node.get('value')

    return f"(* Unsupported expression node type: {node_type} *)"


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
        set(),
        'once',
        node.get('body'),
    )
    return \
        f"Definition {name}{params} : M.t " + \
        function_result_type(len(node.get('returnVariables', []))) + " :=\n" + \
        indent(body(result)) + "."


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
    all_functions = set()
    for function, called_functions in sorted(functions.items()):
        all_functions.add(function)
        for called in called_functions:
            graph[function].append(called)
            all_functions.add(called)

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
    for function in sorted(all_functions):
        if function not in visited:
            dfs(function, visited, stack, set())

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
            indent(block_to_coq(
                set(),
                'once',
                node,
            )[0]("M.pure tt")) + "."
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
