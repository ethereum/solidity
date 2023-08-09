contract test {
    /// @notice Multiplies `a` by 7
    /// and then adds `b`
    function mul_and_add(uint a, uint256 b) public returns (uint256 d) {
        return (a * 7) + b;
    }
}

// ----
// ----
// :test userdoc
// {
//     "methods":
//     {
//         "mul_and_add(uint256,uint256)":
//         {
//             "notice": "Multiplies `a` by 7 and then adds `b`"
//         }
//     }
// }
