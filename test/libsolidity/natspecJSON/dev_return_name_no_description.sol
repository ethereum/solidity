contract A {
    /// @return a
    function g(int x) public pure virtual returns (int a) { return 2; }
}

contract B is A {
    function g(int x) public pure override returns (int b) { return 2; }
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
//                 "a": "a"
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
//                 "b": "a"
//             }
//         }
//     }
// }
