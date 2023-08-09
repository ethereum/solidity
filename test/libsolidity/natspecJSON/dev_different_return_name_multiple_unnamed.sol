contract A {
    /// @return value A
    /// @return value B
    function g(int x) public pure virtual returns (int, int) { return (1, 2); }
}

contract B is A {
    function g(int x) public pure override returns (int z, int y) { return (1, 2); }
}

// ----
// ----
// :A devdoc
// {
//     "kind": "dev",
//     "methods":
//     {
//         "g(int256)":
//         {
//             "returns":
//             {
//                 "_0": "value A",
//                 "_1": "value B"
//             }
//         }
//     },
//     "version": 1
// }
//
// :B devdoc
// {
//     "kind": "dev",
//     "methods":
//     {
//         "g(int256)":
//         {
//             "returns":
//             {
//                 "y": "value B",
//                 "z": "value A"
//             }
//         }
//     },
//     "version": 1
// }
