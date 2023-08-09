contract A {
    /// @return y value
    function g(int x) public pure virtual returns (int y) { return x; }
}

contract B is A {
    function g(int x) public pure override returns (int z) { return x; }
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
//                 "y": "value"
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
//                 "z": "value"
//             }
//         }
//     }
// }
