from collections import defaultdict
import json
from pathlib import Path
import sys

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

def variables_names_to_coq(as_pattern: bool, variable_names) -> str:
    if len(variable_names) == 1:
        return variable_name_to_coq(variable_names[0].get('name'))
    else:
        quote = "'" if as_pattern else ""
        return quote + f"({', '.join(variable_name_to_coq(variable_name.get('name')) for variable_name in variable_names)})"

def node_in_block_to_coq(level: int, node):
    node_type = node.get('nodeType')

    if node_type in ['YulVariableDeclaration', 'YulAssignment']:
        return node_to_coq(level, node)

    elif node_type in ['YulIf', 'YulSwitch']:
        return \
            "do~ [[\n" + \
            indent(node_to_coq(level + 1, node)) + "\n" + \
            "]] in"

    elif node_type in ['YulBlock', 'YulForLoop']:
        return \
            "do~\n" + \
            indent(node_to_coq(level + 1, node)) + "\n" + \
            "in"

    return \
        "do~ [[ " + node_to_coq(level, node) + " ]] in"

def block_to_coq(level: int, node, result: str) -> str:
    node_type = node.get('nodeType')

    if node_type == 'YulBlock':
        statements = [
            node_in_block_to_coq(level, stmt)
            for stmt in node.get('statements', [])
            if stmt.get('nodeType') != 'YulFunctionDefinition'
        ] + [result]
        return "\n".join(statements)

    return "(* Unsupported block node type: {node_type} *)"

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

def node_to_coq(level: int, node) -> str:
    if isinstance(node, dict):
        node_type = node.get('nodeType')

        if node_type == 'YulBlock':
            return block_to_coq(level, node, "M.pure tt")

        elif node_type == 'YulFunctionDefinition':
            return "(* Function definition should be handled at top level *)"

        elif node_type == 'YulVariableDeclaration':
            variables = variables_names_to_coq(True, node.get('variables', []))
            value = node_to_coq(level + 1, node.get('value'))
            return f"let~ {variables} := [[ {value} ]] in"

        elif node_type == 'YulAssignment':
            variable = variables_names_to_coq(True, node.get('variableNames'))
            value = node_to_coq(level + 1, node.get('value'))
            return f"let~ {variable} := [[ {value} ]] in"

        elif node_type == 'YulFunctionCall':
            func_name = variable_name_to_coq(node['functionName']['name'])
            is_pure = is_function_pure(func_name)
            if is_pure:
                func_name = "Pure." + func_name
            args: list[str] = [
                paren(
                    arg.get('nodeType') not in ['YulLiteral', 'YulIdentifier'],
                    node_to_coq(level + 1, arg),
                )
                for arg in node.get('arguments', [])
            ]
            if is_pure:
                return func_name + "".join(" " + arg for arg in args)
            args_left = "~(|"
            args_right = "|)"
            return \
                func_name + " " + \
                (args_left + args_right \
                if len(node.get('arguments', [])) == 0 \
                else \
                    args_left + " " + \
                    ', '.join(args) + \
                    " " + args_right)

        elif node_type == 'YulIdentifier':
            return variable_name_to_coq(node.get('name', 'Unknown identifier'))

        elif node_type == 'YulLiteral':
            if node['kind'] == 'string':
                return "0x" + node['hexValue'].ljust(64, '0')
            return node.get('value', 'Unknown literal')

        elif node_type == 'YulExpressionStatement':
            return node_to_coq(level + 1, node.get('expression'))

        elif node_type == 'YulIf':
            condition = node_to_coq(level, node.get('condition'))
            true_body = node_to_coq(level + 1, node.get('body'))
            return \
                f"M.if_unit (| {condition},\n" + \
                indent(true_body) + "\n" + \
                "|)"

        elif node_type == 'YulSwitch':
            expression = node_to_coq(level, node.get('expression'))
            cases = [
                f"if δ =? {node_to_coq(level, case.get('value'))} then\n" + \
                indent(node_to_coq(level + 1, case.get('body')))
                for case in node.get('cases', [])
            ]
            return \
                "(* switch *)\n" + \
                f"let* δ := ltac:(M.monadic ({expression})) in\n" + \
                ("\nelse ").join(cases) + "\n" + \
                "else\n" + \
                indent("M.pure tt")

        elif node_type == 'YulLeave':
            return "M.leave"

        elif node_type == 'YulBreak':
            return "M.break"

        elif node_type == 'YulContinue':
            return "M.continue"

        elif node_type == 'YulForLoop':
            pre = node_in_block_to_coq(level, node.get('pre'))
            condition = node_to_coq(level + 1, node.get('condition'))
            post = node_to_coq(level + 1, node.get('post'))
            body = node_to_coq(level + 1, node.get('body'))

            return \
                "(* for loop *)\n" + \
                "(* pre *)\n" + \
                pre + "\n" + \
                "M.for_unit\n" + \
                indent(
                    "(* condition *)\n" + \
                    "[[ " + condition + " ]]\n" + \
                    "(* body *)\n" + \
                    "(" + body + ")\n" + \
                    "(* post *)\n" + \
                    "(" + post + ")"
                )

        else:
            return f"(* Unsupported node type: {node_type} *)"

    else:
        # A node should always be a dictionary
        return f"(* Unsupported node: {node} *)"

def function_result_value(returnVariables) -> str:
    if len(returnVariables) == 0:
        return "M.pure tt"

    return "M.pure " + variables_names_to_coq(False, returnVariables)

def function_result_type(arity: int) -> str:
    if arity == 0:
        return "unit"
    elif arity == 1:
        return "U256.t"

    return "(" + " * ".join(["U256.t"] * arity) + ")"

def function_definition_to_coq(level: int, node) -> str:
    name = variable_name_to_coq(node.get('name'))
    params = ''.join([
        " (" + variable_name_to_coq(p['name']) + " : U256.t)"
        for p in node.get('parameters', [])
    ])
    result = function_result_value(node.get('returnVariables', []))
    body = block_to_coq(level + 1, node.get('body'), result)
    return \
        f"Definition {name}{params} : M.t {function_result_type(len(node.get('returnVariables', [])))} :=\n" + \
        indent(body + ".")

# Get the names of the functions called in a function.
# We take care of sorting the names in alphabetical order so that the output is deterministic.
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
    name_order: dict[str, int] = {name: index for index, name in enumerate(ordered_names)}

    # Define a key function that returns the index of the function name in ordered_names
    def key_func(node):
        return name_order.get(node.get('name'), len(ordered_names))  # Put unknown names at the end

    # Sort the function_nodes using the key function
    return sorted(function_nodes, key=key_func)

def top_level_to_coq(level: int, node) -> str:
    node_type = node.get('nodeType')

    if node_type == 'YulBlock':
        functions_dependencies: dict[str, list[str]] = {}
        for statement in node.get('statements', []):
            if statement.get('nodeType') == 'YulFunctionDefinition':
                function_name = statement.get('name')
                dependencies = get_function_dependencies(statement)
                functions_dependencies[function_name] = dependencies
        ordered_function_names = topological_sort(functions_dependencies)
        functions = [
            function_definition_to_coq(level, stmt)
            for stmt in order_functions(ordered_function_names, node.get('statements', []))
            if stmt.get('nodeType') == 'YulFunctionDefinition'
        ]
        body = \
            "Definition body : M.t unit :=\n" + \
            indent(node_to_coq(level + 1, node)) + "."
        return ("\n\n").join(functions + [body])

    return f"(* Unsupported top-level node type: {node_type} *)"

def object_to_coq(level: int, node) -> str:
    node_type = node.get('nodeType')

    if node_type == 'YulObject':
        return \
            "Module " + node['name'] + ".\n" + \
            indent(object_to_coq(level + 1, node['code'])) + "\n" + \
            "".join(
                "\n" +
                indent(object_to_coq(level + 1, child)) + "\n"
                for child in node.get('subObjects', [])
                if child.get('nodeType') != 'YulData'
            ) + \
            "End " + node['name'] + "."

    elif node_type == 'YulCode':
        return top_level_to_coq(level, node['block'])

    elif node_type == 'YulData':
        return "(* Data object not expected *)"

    return f"(* Unsupported object node type: {node_type} *)"

def main():
    """Input: JSON file with Yul AST"""
    with open(sys.argv[1], 'r') as file:
        data = json.load(file)

    coq_code = object_to_coq(0, data)

    print("(* Generated by " + Path(__file__).name + " *)")
    print("Require Import CoqOfSolidity.CoqOfSolidity.")
    print("Require Import CoqOfSolidity.simulations.CoqOfSolidity.")
    print("Import Stdlib.")
    print()
    print(coq_code)

if __name__ == "__main__":
    main()
