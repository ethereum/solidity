contract test {
    /// Something failed.
    /// @dev an error.
    /// @param a first parameter
    /// @param b second parameter
    error E(uint a, uint b);
}

// ----
// ----
// :test devdoc
// {
//     "errors":
//     {
//         "E(uint256,uint256)":
//         [
//             {
//                 "details": "an error.",
//                 "params":
//                 {
//                     "a": "first parameter",
//                     "b": "second parameter"
//                 }
//             }
//         ]
//     },
//     "methods": {}
// }
//
// :test userdoc
// {
//     "errors":
//     {
//         "E(uint256,uint256)":
//         [
//             {
//                 "notice": "Something failed."
//             }
//         ]
//     },
//     "methods": {}
// }
