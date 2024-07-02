from collections import defaultdict
import json
import sys

def indent(level):
    return "  " * level

def paren(condition: bool, value: str) -> str:
    return f"({value})" if condition else value

def variable_name_to_coq(name: str) -> str:
    reserved_names = [
        "end",
        "return",
    ]
    if name in reserved_names:
        return name + "_"
    return name

def variables_names_to_coq(as_pattern: bool, variable_names) -> str:
    if len(variable_names) == 1:
        return variable_name_to_coq(variable_names[0].get('name'))
    else:
        quote = "'" if as_pattern else ""
        return quote + f"({', '.join(variable_name_to_coq(variable_name.get('name')) for variable_name in variable_names)})"

def node_in_block_to_coq(local_functions: list[str], level: int, node):
    node_type = node.get('nodeType')

    if node_type in ['YulVariableDeclaration', 'YulAssignment']:
        return node_to_coq(local_functions, level, node)

    elif node_type in ['YulBlock', 'YulIf', 'YulSwitch']:
        return \
            "do~ [[\n" + \
            indent(level + 1) + node_to_coq(local_functions, level + 1, node) + "\n" + \
            indent(level) + "]] in"

    return \
        "do~ [[ " + node_to_coq(local_functions, level, node) + " ]] in"

def block_to_coq(local_functions: list[str], level: int, node, result: str) -> str:
    node_type = node.get('nodeType')

    if node_type == 'YulBlock':
        statements = [
            node_in_block_to_coq(local_functions, level, stmt)
            for stmt in node.get('statements', [])
            if stmt.get('nodeType') != 'YulFunctionDefinition'
        ]
        return \
            ("\n" + indent(level)).join(statements) + "\n" + \
            indent(level) + result

    return "(* Unsupported block node type: {node_type} *)"

def is_function_pure(function_name: str) -> bool:
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

def node_to_coq(local_functions: list[str], level: int, node) -> str:
    if isinstance(node, dict):
        node_type = node.get('nodeType')

        if node_type == 'YulBlock':
            return block_to_coq(local_functions, level, node, "M.pure tt")

        elif node_type == 'YulFunctionDefinition':
            return "(* Function definition should be handled at top level *)"

        elif node_type == 'YulVariableDeclaration':
            variables = variables_names_to_coq(True, node.get('variables', []))
            value = node_to_coq(local_functions, level + 1, node.get('value'))
            return f"let~ {variables} := [[ {value} ]] in"

        elif node_type == 'YulAssignment':
            variable = variables_names_to_coq(True, node.get('variableNames'))
            value = node_to_coq(local_functions, level + 1, node.get('value'))
            return f"let~ {variable} := [[ {value} ]] in"

        elif node_type == 'YulFunctionCall':
            func_name = variable_name_to_coq(node['functionName']['name'])
            is_pure = is_function_pure(func_name)
            if is_pure:
                func_name = "Pure." + func_name
            args: list[str] = [
                paren(
                    arg.get('nodeType') not in ['YulLiteral', 'YulIdentifier'],
                    node_to_coq(local_functions, level + 1, arg),
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
            return node_to_coq(local_functions, level + 1, node.get('expression'))

        elif node_type == 'YulIf':
            condition = node_to_coq(local_functions, level, node.get('condition'))
            true_body = node_to_coq(local_functions, level + 1, node.get('body'))
            return \
                f"M.if_unit (| {condition},\n" + \
                indent(level + 1) + true_body + "\n" + \
                indent(level) + "|)"

        elif node_type == 'YulSwitch':
            expression = node_to_coq(local_functions, level, node.get('expression'))
            cases = [
                f"if δ =? {node_to_coq(local_functions, level, case.get('value'))} then\n" + \
                indent(level + 1) + node_to_coq(local_functions, level + 1, case.get('body'))
                for case in node.get('cases', [])
            ]
            return \
                "(* switch *)\n" + \
                indent(level) + f"let* δ := ltac:(M.monadic ({expression})) in\n" + \
                indent(level) + ("\n" + indent(level) + "else ").join(cases) + "\n" + \
                indent(level) + "else\n" + \
                indent(level + 1) + "M.pure tt"

        elif node_type == 'YulLeave':
            return "M.leave"

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

def function_definition_to_coq(local_functions: list[str], level: int, node) -> str:
    name = node.get('name')
    params = ''.join([
        " (" + variable_name_to_coq(p['name']) + " : U256.t)"
        for p in node.get('parameters', [])
    ])
    result = function_result_value(node.get('returnVariables', []))
    body = block_to_coq(local_functions, level + 1, node.get('body'), result)
    return \
        f"Definition {name}{params} : M.t {function_result_type(len(node.get('returnVariables', [])))} :=\n" + \
        indent(level + 1) + body + "."

def get_function_dependencies(function_node) -> list[str]:
    dependencies = set()

    def traverse(node):
        if isinstance(node, dict):
            if node.get('nodeType') == 'YulFunctionCall':
                function_name = node['functionName']['name']
                dependencies.add(function_name)
            for value in node.values():
                traverse(value)
        elif isinstance(node, list):
            for item in node:
                traverse(item)

    # Start traversal from the 'statements' field
    traverse(function_node.get('body', {}))

    return list(dependencies)

def topological_sort(functions: dict[str, list[str]]) -> list[str]:
    # Create a graph representation
    graph = defaultdict(list)
    all_funcs = set()
    for func, called_funcs in functions.items():
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
    for func in all_funcs:
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
        local_functions = topological_sort(functions_dependencies)
        functions = [
            function_definition_to_coq(local_functions, level, stmt)
            for stmt in order_functions(local_functions, node.get('statements', []))
            if stmt.get('nodeType') == 'YulFunctionDefinition'
        ]
        body = node_to_coq(local_functions, level + 1, node)
        return \
            ("\n\n" + indent(level)).join(functions) + "\n\n" + \
            indent(level) + "Definition body : M.t unit :=\n" + \
            indent(level + 1) + body + "."

    return f"(* Unsupported top-level node type: {node_type} *)"

def object_to_coq(level: int, node) -> str:
    node_type = node.get('nodeType')

    if node_type == 'YulObject':
        return \
            "Module " + node['name'] + ".\n" + \
            indent(level + 1) + object_to_coq(level + 1, node['code']) + "\n" + \
            "".join(
                "\n" +
                indent(level + 1) + object_to_coq(level + 1, child) + "\n"
                for child in node.get('subObjects', [])
                if child.get('nodeType') != 'YulData'
            ) + \
            indent(level) + "End " + node['name'] + "."

    elif node_type == 'YulCode':
        return top_level_to_coq(level, node['block'])

    elif node_type == 'YulData':
        return "(* Data object not expected *)"

    return f"(* Unsupported object node type: {node_type} *)"

def main():
    with open(sys.argv[1], 'r') as file:
        data = json.load(file)

    coq_code = object_to_coq(0, data)

    print("(* Generated by prepare.py *)")
    print("Require Import CoqOfSolidity.CoqOfSolidity.")
    print("Require Import simulations.CoqOfSolidity.")
    print("Import Stdlib.")
    print()
    print(coq_code)

if __name__ == "__main__":
    main()
