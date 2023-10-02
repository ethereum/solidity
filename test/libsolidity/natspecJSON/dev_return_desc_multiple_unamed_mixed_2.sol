contract test {
    /// @dev Multiplies a number by 7 and adds second parameter
    /// @param a Documentation for the first parameter starts here.
    /// Since it's a really complicated parameter we need 2 lines
    /// @param second Documentation for the second parameter
    /// @return _cookies And cookies with nutella
    /// @return The result of the multiplication
    /// @return _milk And milk with nutella
    function mul(uint a, uint second) public returns (uint _cookies, uint, uint _milk) {
        uint mul = a * 7;
        uint milk = 4;
        return (mul, second, milk);
    }
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
//                 "a": "Documentation for the first parameter starts here. Since it's a really complicated parameter we need 2 lines",
//                 "second": "Documentation for the second parameter"
//             },
//             "returns":
//             {
//                 "_1": "The result of the multiplication",
//                 "_cookies": "And cookies with nutella",
//                 "_milk": "And milk with nutella"
//             }
//         }
//     },
//     "version": 1
// }
//
// :test userdoc
// {
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
