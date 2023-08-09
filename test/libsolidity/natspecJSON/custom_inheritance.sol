contract A {
    /// @custom:since 2014
    function g(uint x) public pure virtual {}
}
contract B is A {
    function g(uint x) public pure override {}
}

// ----
// ----
// :A devdoc
// {
//     "kind": "dev",
//     "methods":
//     {
//         "g(uint256)":
//         {
//             "custom:since": "2014"
//         }
//     },
//     "version": 1
// }
//
// :B devdoc
// {
//     "kind": "dev",
//     "methods": {},
//     "version": 1
// }
