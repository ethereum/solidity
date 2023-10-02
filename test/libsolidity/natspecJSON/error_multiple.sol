contract A {
    /// Something failed.
    /// @dev an error.
    /// @param x first parameter
    /// @param y second parameter
    error E(uint x, uint y);
}
contract test {
    /// X Something failed.
    /// @dev X an error.
    /// @param a X first parameter
    /// @param b X second parameter
    error E(uint a, uint b);
    function f(bool a) public pure {
        if (a)
            revert E(1, 2);
        else
            revert A.E(5, 6);
    }
}

// ----
// ----
// :A devdoc
// {
//     "errors":
//     {
//         "E(uint256,uint256)":
//         [
//             {
//                 "details": "an error.",
//                 "params":
//                 {
//                     "x": "first parameter",
//                     "y": "second parameter"
//                 }
//             }
//         ]
//     },
//     "kind": "dev",
//     "methods": {},
//     "version": 1
// }
//
// :A userdoc
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
//
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
//                     "x": "first parameter",
//                     "y": "second parameter"
//                 }
//             },
//             {
//                 "details": "X an error.",
//                 "params":
//                 {
//                     "a": "X first parameter",
//                     "b": "X second parameter"
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
//             },
//             {
//                 "notice": "X Something failed."
//             }
//         ]
//     },
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
