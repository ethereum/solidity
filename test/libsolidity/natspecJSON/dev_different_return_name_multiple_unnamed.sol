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
//     }
// }
//
// :B devdoc
// {
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
//     }
// }
