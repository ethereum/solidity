contract test {
    /// @dev
    /// Multiplies a number by 7 and adds second parameter
    /// @param a Documentation for the first parameter
    /// @param second Documentation for the second parameter
    function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
}

// ----
// ----
// :test devdoc
// {
//     "kind": "dev",
//     "methods":
//     {
//         "mul(uint256,uint256)":
//         {
//             "details": "Multiplies a number by 7 and adds second parameter",
//             "params":
//             {
//                 "a": "Documentation for the first parameter",
//                 "second": "Documentation for the second parameter"
//             }
//         }
//     },
//     "version": 1
// }
