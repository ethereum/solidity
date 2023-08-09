contract A {
    /// @return a value A
    /// @return b value B
    function g(int x) public pure virtual returns (int a, int b) { return (1, 2); }
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
//                 "a": "value A",
//                 "b": "value B"
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
