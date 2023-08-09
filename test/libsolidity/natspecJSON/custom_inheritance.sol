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
//     "methods":
//     {
//         "g(uint256)":
//         {
//             "custom:since": "2014"
//         }
//     }
// }
//
// :B devdoc
// {
//     "methods": {}
// }
