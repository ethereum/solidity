contract test {
    /// @return d The result of the multiplication
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
//             "returns":
//             {
//                 "d": "The result of the multiplication"
//             }
//         }
//     }
// }
