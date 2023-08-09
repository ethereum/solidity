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
//     "kind": "dev",
//     "methods": {},
//     "version": 1
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
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
