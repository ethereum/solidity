contract test {
    /// @notice Multiplies `a` by 7 and then adds `b`
    function mul_and_add(uint a, uint256 b) public returns (uint256 d) {
        return (a * 7) + b;
    }

    /// @notice Divides `input` by `div`
    function divide(uint input, uint div) public returns (uint d) {
        return input / div;
    }

    /// @notice Subtracts 3 from `input`
    function sub(int input) public returns (int d) {
        return input - 3;
    }
}

// ----
// ----
// :test userdoc
// {
//     "methods":
//     {
//         "divide(uint256,uint256)":
//         {
//             "notice": "Divides `input` by `div`"
//         },
//         "mul_and_add(uint256,uint256)":
//         {
//             "notice": "Multiplies `a` by 7 and then adds `b`"
//         },
//         "sub(int256)":
//         {
//             "notice": "Subtracts 3 from `input`"
//         }
//     }
// }
