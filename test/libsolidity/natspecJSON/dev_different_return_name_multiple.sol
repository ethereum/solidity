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
//     "kind": "dev",
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
//     },
//     "version": 1
// }
//
// :A userdoc
// {
//     "kind": "user",
//     "methods": {},
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
//
// :B userdoc
// {
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
