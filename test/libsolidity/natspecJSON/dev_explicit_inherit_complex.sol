==== Source: Interfaces.sol ====
interface ERC20 {
    /// Transfer ``amount`` from ``msg.sender`` to ``to``.
    /// @dev test
    /// @param to address to transfer to
    /// @param amount amount to transfer
    function transfer(address to, uint amount) external returns (bool);
}

interface ERC21 {
    /// Transfer ``amount`` from ``msg.sender`` to ``to``.
    /// @dev test2
    /// @param to address to transfer to
    /// @param amount amount to transfer
    function transfer(address to, uint amount) external returns (bool);
}

==== Source: Testfile.sol ====
import "Interfaces.sol" as myInterfaces;

contract Token is myInterfaces.ERC20, myInterfaces.ERC21 {
    /// @inheritdoc myInterfaces.ERC20
    function transfer(address too, uint amount)
        override(myInterfaces.ERC20, myInterfaces.ERC21) external returns (bool) {
        return false;
    }
}

// ----
// ----
// Testfile.sol:Token devdoc
// {
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
//     }
// }
