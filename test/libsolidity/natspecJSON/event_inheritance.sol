contract ERC20 {
    /// @notice This event is emitted when a transfer occurs.
    /// @param from The source account.
    /// @param to The destination account.
    /// @param amount The amount.
    /// @dev A test case!
    event Transfer(address indexed from, address indexed to, uint amount);
}
contract A is ERC20 {
}
contract B is A {
}

// ----
// ----
// :A devdoc
// {
//     "events":
//     {
//         "Transfer(address,address,uint256)":
//         {
//             "details": "A test case!",
//             "params":
//             {
//                 "amount": "The amount.",
//                 "from": "The source account.",
//                 "to": "The destination account."
//             }
//         }
//     },
//     "kind": "dev",
//     "methods": {},
//     "version": 1
// }
//
// :A userdoc
// {
//     "events":
//     {
//         "Transfer(address,address,uint256)":
//         {
//             "notice": "This event is emitted when a transfer occurs."
//         }
//     },
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
//
// :B devdoc
// {
//     "events":
//     {
//         "Transfer(address,address,uint256)":
//         {
//             "details": "A test case!",
//             "params":
//             {
//                 "amount": "The amount.",
//                 "from": "The source account.",
//                 "to": "The destination account."
//             }
//         }
//     },
//     "kind": "dev",
//     "methods": {},
//     "version": 1
// }
//
// :B userdoc
// {
//     "events":
//     {
//         "Transfer(address,address,uint256)":
//         {
//             "notice": "This event is emitted when a transfer occurs."
//         }
//     },
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
//
// :ERC20 devdoc
// {
//     "events":
//     {
//         "Transfer(address,address,uint256)":
//         {
//             "details": "A test case!",
//             "params":
//             {
//                 "amount": "The amount.",
//                 "from": "The source account.",
//                 "to": "The destination account."
//             }
//         }
//     },
//     "kind": "dev",
//     "methods": {},
//     "version": 1
// }
//
// :ERC20 userdoc
// {
//     "events":
//     {
//         "Transfer(address,address,uint256)":
//         {
//             "notice": "This event is emitted when a transfer occurs."
//         }
//     },
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
