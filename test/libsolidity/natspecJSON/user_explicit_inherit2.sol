interface ERC20 {
    /// Transfer ``amount`` from ``msg.sender`` to ``to``.
    /// @dev test
    /// @param to address to transfer to
    /// @param amount amount to transfer
    function transfer(address to, uint amount) external returns (bool);
}

contract ERC21 is ERC20 {
    function transfer(address to, uint amount) virtual override external returns (bool) {
        return false;
    }
}

contract Token is ERC20 {
    /// @inheritdoc ERC20
    function transfer(address to, uint amount) override external returns (bool) {
        return false;
    }
}

// ----
// ----
// :ERC20 userdoc
// {
//     "methods":
//     {
//         "transfer(address,uint256)":
//         {
//             "notice": "Transfer ``amount`` from ``msg.sender`` to ``to``."
//         }
//     }
// }
//
// :ERC21 userdoc
// {
//     "methods":
//     {
//         "transfer(address,uint256)":
//         {
//             "notice": "Transfer ``amount`` from ``msg.sender`` to ``to``."
//         }
//     }
// }
//
// :Token userdoc
// {
//     "methods":
//     {
//         "transfer(address,uint256)":
//         {
//             "notice": "Transfer ``amount`` from ``msg.sender`` to ``to``."
//         }
//     }
// }
