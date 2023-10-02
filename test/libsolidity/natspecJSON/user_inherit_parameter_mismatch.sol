interface ERC20 {
    /// Transfer ``amount`` from ``msg.sender`` to ``to``.
    /// @dev test
    /// @param to address to transfer to
    /// @param amount amount to transfer
    function transfer(address to, uint amount) external returns (bool);
}

contract Middle is ERC20 {
    function transfer(address to, uint amount) override virtual external returns (bool) {
        return false;
    }
}

contract Token is Middle {
    function transfer(address too, uint amount) override external returns (bool) {
        return false;
    }
}

// ----
// ----
// :ERC20 devdoc
// {
//     "kind": "dev",
//     "methods":
//     {
//         "transfer(address,uint256)":
//         {
//             "details": "test",
//             "params":
//             {
//                 "amount": "amount to transfer",
//                 "to": "address to transfer to"
//             }
//         }
//     },
//     "version": 1
// }
//
// :ERC20 userdoc
// {
//     "kind": "user",
//     "methods":
//     {
//         "transfer(address,uint256)":
//         {
//             "notice": "Transfer ``amount`` from ``msg.sender`` to ``to``."
//         }
//     },
//     "version": 1
// }
//
// :Middle devdoc
// {
//     "kind": "dev",
//     "methods":
//     {
//         "transfer(address,uint256)":
//         {
//             "details": "test",
//             "params":
//             {
//                 "amount": "amount to transfer",
//                 "to": "address to transfer to"
//             }
//         }
//     },
//     "version": 1
// }
//
// :Middle userdoc
// {
//     "kind": "user",
//     "methods":
//     {
//         "transfer(address,uint256)":
//         {
//             "notice": "Transfer ``amount`` from ``msg.sender`` to ``to``."
//         }
//     },
//     "version": 1
// }
//
// :Token devdoc
// {
//     "kind": "dev",
//     "methods": {},
//     "version": 1
// }
//
// :Token userdoc
// {
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
