contract C {
    /// @notice Hello world
    /// @dev test
    function x() virtual external returns (uint) {
        return 1;
    }
}

contract D is C {
    uint public override x;
}

// ----
// ----
// :C userdoc
// {
//     "methods":
//     {
//         "x()":
//         {
//             "notice": "Hello world"
//         }
//     }
// }
//
// :D userdoc
// {
//     "methods":
//     {
//         "x()":
//         {
//             "notice": "Hello world"
//         }
//     }
// }
