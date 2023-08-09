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
//     "kind": "dev",
//     "methods":
//     {
//         "g(int256)":
//         {
//             "returns":
//             {
//                 "y": "value"
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
//                 "z": "value"
//             }
//         }
//     },
//     "version": 1
// }
