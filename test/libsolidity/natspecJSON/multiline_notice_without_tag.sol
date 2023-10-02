contract test {
    /// I do something awesome
    /// which requires two lines to explain
    function mul(uint a) public returns (uint d) { return a * 7; }
}

// ----
// ----
// :test devdoc
// {
//     "kind": "dev",
//     "methods": {},
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
//             "notice": "I do something awesome which requires two lines to explain"
//         }
//     },
//     "version": 1
// }
