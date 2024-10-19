from collections import defaultdict
import json
from pathlib import Path
import sys
from typing import Callable, Union


# Indent each line of the block, except empty lines
def indent(block: str) -> str:
    indentation = "  "
    return "\n".join(
        line if line == "" else indentation + line
        for line in block.split("\n")
    )


def paren(condition: bool, value: str) -> str:
    return f"({value})" if condition else value


def name_to_coq(name: str) -> str:
    reserved_names = [
        "end",
        "return",
    ]

    if name in reserved_names:
        return name + "_"

    return name.replace("$", "'dollar'")


def names_to_coq(as_pattern: bool, names: list[str]) -> str:
    if len(names) == 0:
        return "'tt" if as_pattern else "tt"

    if len(names) == 1:
        return name_to_coq(names[0])

    quote = "'" if as_pattern else ""
    return \
        quote + "(" + \
        ', '.join(name_to_coq(name) for name in names) + \
        ")"


def variable_name_to_name(variable_name) -> str:
    return variable_name.get('name')


def variable_name_to_coq(variable_name) -> str:
    return name_to_coq(variable_name_to_name(variable_name))


def variable_names_to_coq(as_pattern: bool, variable_names: list) -> str:
    return names_to_coq(
        as_pattern,
        [variable_name_to_name(variable_name) for variable_name in variable_names]
    )


def updated_vars_to_coq(as_pattern: bool, updated_vars: set[str]) -> str:
    return names_to_coq(as_pattern, sorted(updated_vars))


def block_to_coq(
    return_variables: Union[None, list],
    node,
) -> tuple[str, set[str]]:
    node_type = node.get('nodeType')

    if node_type == 'YulBlock':
        nodes = node.get('statements', [])
        statements: list[Callable[[set[str]], str]] = []
        return_mode = "Tt"
        declared_vars: set[str] = set()
        updated_vars: set[str] = set()

        if len(nodes) > 0:
            last_node = nodes[-1]
            if isinstance(last_node, dict):
                last_node_type = last_node.get('nodeType')
                if last_node_type == 'YulBreak':
                    return_mode = "Break"
                    nodes = nodes[:-1]
                elif last_node_type == 'YulContinue':
                    return_mode = "Continue"
                    nodes = nodes[:-1]
                elif last_node_type == 'YulLeave':
                    return_mode = "Leave"
                    nodes = nodes[:-1]

        for node in nodes:
            # We ignore the functions here as they are handled separately
            if node.get('nodeType') == 'YulFunctionDefinition':
                continue

            statement, statement_declared_vars, statement_updated_vars = \
                statement_to_coq(node)
            statements += [statement]
            declared_vars |= statement_declared_vars
            updated_vars |= statement_updated_vars

        updated_vars -= declared_vars

        if return_variables is not None:
            suffix = "M.pure " + variable_names_to_coq(False, return_variables)
        else:
            suffix = \
                "M.pure (BlockUnit." + return_mode + ", " + \
                updated_vars_to_coq(False, updated_vars) + ")"

        return (
            "\n".join(
                [statement(updated_vars) for statement in statements] +
                [suffix]
            ),
            updated_vars,
        )

    return (
        "(* Unsupported block node type: {node_type} *)",
        set(),
    )


def is_pre_empty_block(node) -> bool:
    return node.get('nodeType') == 'YulBlock' and len(node.get('statements', [])) == 0


def lift_state_update(
    block: str,
    local_updated_vars: set[str],
    global_updated_vars: set[str]
) -> str:
    if local_updated_vars == global_updated_vars:
        return block

    return \
        "Shallow.lift_state_update\n" + \
        indent(
            "(fun " + updated_vars_to_coq(True, local_updated_vars) + \
            " => " + updated_vars_to_coq(False, global_updated_vars) + ")"
        ) + "\n" + \
        indent("(" + block + ")")


def statement_to_coq(node) -> tuple[Callable[[set[str]], str], set[str], set[str]]:
    node_type = node.get('nodeType')

    if node_type == 'YulBlock':
        statement, statement_updated_vars = block_to_coq(None, node)
        return (
            lambda _:
                "do~\n" + \
                indent(statement) + "\n" + \
                "in",
            set(),
            statement_updated_vars,
        )

    elif node_type == 'YulFunctionDefinition':
        # We ignore this case because we only handle top-level function definitions
        return (
            lambda _:
                "(* Function definition not expected at block level*)",
            set(),
            set(),
        )

    elif node_type == 'YulVariableDeclaration':
        variable_names = node.get('variables', [])
        variables = variable_names_to_coq(True, variable_names)
        value = \
            expression_to_coq(node.get('value')) \
            if node.get('value') is not None \
            else "0"
        return (
            lambda _:
                f"let~ {variables} := [[ {value} ]] in",
            {
                variable_name.get('name')
                for variable_name in variable_names
            },
            set(),
        )

    elif node_type == 'YulAssignment':
        variable_names = node.get('variableNames', [])
        variables = variable_names_to_coq(True, variable_names)
        value = expression_to_coq(node.get('value'))
        return (
            lambda _:
                f"let~ {variables} := [[ {value} ]] in",
            set(),
            {
                variable_name.get('name')
                for variable_name in variable_names
            },
        )

    elif node_type == 'YulExpressionStatement':
        return (
            lambda _:
                "do~ [[ " + expression_to_coq(node.get('expression')) + " ]] in",
            set(),
            set(),
        )

    elif node_type == 'YulIf':
        condition = expression_to_coq(node.get('condition'))
        then_body, then_updated_vars = block_to_coq(None, node.get('body'))
        return (
            lambda final_updated_vars:
                "let_state~ " + \
                updated_vars_to_coq(True, then_updated_vars) + \
                " := [[\n" + \
                indent(
                    "Shallow.if_ (|\n" +
                    indent(condition) + ",\n" +
                    indent(then_body) + ",\n" +
                    indent(updated_vars_to_coq(False, then_updated_vars)) + "\n" +
                    "|)"
                ) + "\n" + \
                "]] default~ " + updated_vars_to_coq(False, final_updated_vars) + " in",
            set(),
            then_updated_vars,
        )

    elif node_type == 'YulSwitch':
        expression = expression_to_coq(node.get('expression'))
        cases = [
            (
                expression_to_coq(case.get('value')),
                block_to_coq(None, case.get('body')),
            )
            for case in node.get('cases', [])
            # TODO: handle the default case in a switch
            if case.get('value') != "default"
        ]
        commonly_updated_vars: set[str] = {
            name
            for _, (_, updated_vars) in cases
            for name in updated_vars
        }
        return (
            lambda final_updated_vars:
                "let_state~ " + \
                updated_vars_to_coq(True, final_updated_vars) + \
                " := [[\n" + \
                indent(
                    "(* switch *)\n" + \
                    f"let~ δ := [[ {expression} ]] in\n" + \
                    "\nelse ".join(
                        f"if δ =? {value} then\n" +
                        indent(lift_state_update(
                            body,
                            updated_vars,
                            final_updated_vars,
                        ))
                        for value, (body, updated_vars) in cases
                    ) + "\n" + \
                    "else\n" + \
                    indent(
                        "M.pure (BlockUnit.Tt, " + \
                        updated_vars_to_coq(False, final_updated_vars) + ")"
                    )
                ) + "\n" + \
                "]] in",
            set(),
            commonly_updated_vars,
        )

    elif node_type in ['YulLeave', 'YulBreak', 'YulContinue']:
        return (
            lambda _:
                f"(* Unexpected statement node type: {node_type} *)",
            set(),
            set(),
        )

    elif node_type == 'YulForLoop':
        # We have not yet seen a case where the pre block is not empty, so this is not
        # tested and we prefer to return an exception.
        if not is_pre_empty_block(node.get('pre')):
            # raise Exception("Non-empty pre block in for loop not handled")
            print("Non-empty pre block in for loop not handled", file=sys.stderr)

        condition = expression_to_coq(node.get('condition'))
        post, post_updated_vars = block_to_coq(None, node.get('post'))
        body, body_updated_vars = block_to_coq(None, node.get('body'))
        updated_vars = post_updated_vars | body_updated_vars

        def fun_read_updated_vars(updated_vars: set[str]) -> str:
            return "(fun " + updated_vars_to_coq(True, updated_vars) + " =>"

        return (
            lambda final_updated_vars:
                "let_state~ " + \
                updated_vars_to_coq(True, updated_vars) + \
                " :=\n" + \
                indent(
                    "(* for loop *)\n" + \
                    "Shallow.for_\n" + \
                    indent(
                        "(* init state *)\n" + \
                        updated_vars_to_coq(False, updated_vars) + "\n" + \
                        "(* condition *)\n" + \
                        fun_read_updated_vars(updated_vars) + " [[\n" + \
                        indent(condition) + "\n" + \
                        "]])\n" + \
                        "(* body *)\n" + \
                        fun_read_updated_vars(updated_vars) + "\n" + \
                        indent(lift_state_update(
                            body,
                            body_updated_vars,
                            updated_vars,
                        )) + ")\n" + \
                        "(* post *)\n" + \
                        fun_read_updated_vars(updated_vars) + "\n" + \
                        indent(lift_state_update(
                            post,
                            post_updated_vars,
                            updated_vars,
                        )) + ")"
                    )
                ) + "\n" + \
                "default~ " + updated_vars_to_coq(False, final_updated_vars) + " in",
            set(),
            updated_vars,
        )

    return (
        lambda _:
            f"(* Unsupported statement node type: {node_type} *)",
        set(),
        set(),
    )


def expression_to_coq(node) -> str:
    node_type = node.get('nodeType')

    if node_type == 'YulFunctionCall':
        func_name = variable_name_to_coq(node['functionName'])
        args: list[str] = [expression_to_coq(arg) for arg in node.get('arguments', [])]
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
        return variable_name_to_coq(node)

    if node_type == 'YulLiteral':
        if node['kind'] == 'string':
            return \
                "0x" + node['hexValue'].ljust(64, '0') + \
                " (* " + node['value'] + " *)"
        return node.get('value')

    return f"(* Unsupported expression node type: {node_type} *)"


def function_result_type(arity: int) -> str:
    if arity == 0:
        return "unit"
    elif arity == 1:
        return "U256.t"

    return "(" + " * ".join(["U256.t"] * arity) + ")"


def function_definition_to_coq(node) -> str:
    name = variable_name_to_coq(node)
    param_names: list[str] = [
        variable_name_to_coq(param)
        for param in node.get('parameters', [])
    ]
    params = ''.join([
        " (" + name + " : U256.t)"
        for name in param_names
    ])
    body, _ = block_to_coq(None, node.get('body'))
    return \
        f"Definition {name}{params} : M.t " + \
        function_result_type(len(node.get('returnVariables', []))) + " :=\n" + \
        indent(
            "let~ '(_, result) :=" + "\n" + \
            indent(body) + "\n" + \
            "in\n" + \
            "M.pure result"
        ) + "."


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
            indent(block_to_coq([], node)[0]) + "."
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
