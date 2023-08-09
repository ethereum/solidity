interface ERC20 {
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
//     "methods": {}
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
//     "methods": {}
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
//     "methods": {}
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
//     "methods": {}
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
//     "methods": {}
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
//     "methods": {}
// }
