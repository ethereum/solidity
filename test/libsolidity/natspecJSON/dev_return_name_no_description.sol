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
//     "kind": "dev",
//     "methods":
//     {
//         "g(int256)":
//         {
//             "returns":
//             {
//                 "a": "a"
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
//                 "b": "a"
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
