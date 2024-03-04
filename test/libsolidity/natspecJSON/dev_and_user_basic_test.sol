contract test {
    /// @notice Multiplies `a` by 7
    /// @dev Multiplies a number by 7
    function mul(uint a) public returns (uint d) { return a * 7; }
}

// ----
// ----
// :test devdoc
// {
//     "kind": "dev",
//     "methods":
//     {
//         "mul(uint256)":
//         {
//             "details": "Multiplies a number by 7"
//         }
//     },
//     "version": 1
// }
//
// :test userdoc
// {
//     "kind": "user",
//     "methods":
//     {
//         "mul(uint256)":
//         {
//             "notice": "Multiplies `a` by 7"
//         }
//     },
//     "version": 1
// }
