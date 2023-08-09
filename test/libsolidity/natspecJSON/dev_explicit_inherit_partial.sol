interface ERC20 {
    /// Transfer ``amount`` from ``msg.sender`` to ``to``.
    /// @dev test
    /// @param to address to transfer to
    /// @param amount amount to transfer
    function transfer(address to, uint amount) external returns (bool);
}

contract ERC21 {
    function transfer(address to, uint amount) virtual external returns (bool) {
        return false;
    }
}

contract Token is ERC21, ERC20 {
    /// @inheritdoc ERC20
    /// @dev override dev comment
    /// @notice override notice
    function transfer(address to, uint amount) override(ERC21, ERC20) external returns (bool) {
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
// :Token devdoc
// {
//     "kind": "dev",
//     "methods":
//     {
//         "transfer(address,uint256)":
//         {
//             "details": "override dev comment",
//             "params":
//             {
//                 "amount": "amount to transfer",
//                 "to": "address to transfer to"
//             }
//         }
//     },
//     "version": 1
// }
