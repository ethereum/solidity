contract test {
    /// @notice this is a really nice constructor
    constructor(uint a, uint second) { }
    /// another multiplier
    function mul(uint a, uint second) public returns(uint d) { return a * 7 + second; }
}

// ----
// ----
// :test userdoc
// {
//     "kind": "user",
//     "methods":
//     {
//         "constructor":
//         {
//             "notice": "this is a really nice constructor"
//         },
//         "mul(uint256,uint256)":
//         {
//             "notice": "another multiplier"
//         }
//     },
//     "version": 1
// }
