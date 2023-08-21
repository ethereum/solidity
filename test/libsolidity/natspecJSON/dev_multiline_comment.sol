contract test {
    /**
     * @dev Multiplies a number by 7 and adds second parameter
     * @param a Documentation for the first parameter starts here.
     * Since it's a really complicated parameter we need 2 lines
     * @param second Documentation for the second parameter
     * @return d The result of the multiplication
     * and cookies with nutella
     */
    function mul(uint a, uint second) public returns (uint d) {
        return a * 7 + second;
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
//                 "d": "The result of the multiplication and cookies with nutella"
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
