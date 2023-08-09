contract test {
    /// @dev Mul function
    function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
}

// ----
// ----
// :test devdoc
// {
//     "methods":
//     {
//         "mul(uint256,uint256)":
//         {
//             "details": "Mul function"
//         }
//     }
// }
