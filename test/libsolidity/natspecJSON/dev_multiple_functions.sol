contract test {
    /// @dev Multiplies a number by 7 and adds second parameter
    /// @param a Documentation for the first parameter
    /// @param second Documentation for the second parameter
    function mul(uint a, uint second) public returns (uint d) {
        return a * 7 + second;
    }
    /// @dev Divides 2 numbers
    /// @param input Documentation for the input parameter
    /// @param div Documentation for the div parameter
    function divide(uint input, uint div) public returns (uint d) {
        return input / div;
    }
    /// @dev Subtracts 3 from `input`
    /// @param input Documentation for the input parameter
    function sub(int input) public returns (int d) {
        return input - 3;
    }
}

// ----
// ----
// :test devdoc
// {
//     "methods":
//     {
//         "divide(uint256,uint256)":
//         {
//             "details": "Divides 2 numbers",
//             "params":
//             {
//                 "div": "Documentation for the div parameter",
//                 "input": "Documentation for the input parameter"
//             }
//         },
//         "mul(uint256,uint256)":
//         {
//             "details": "Multiplies a number by 7 and adds second parameter",
//             "params":
//             {
//                 "a": "Documentation for the first parameter",
//                 "second": "Documentation for the second parameter"
//             }
//         },
//         "sub(int256)":
//         {
//             "details": "Subtracts 3 from `input`",
//             "params":
//             {
//                 "input": "Documentation for the input parameter"
//             }
//         }
//     }
// }
