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
// :C devdoc
// {
//     "methods":
//     {
//         "x()":
//         {
//             "details": "test"
//         }
//     }
// }
//
// :D devdoc
// {
//     "methods": {},
//     "stateVariables":
//     {
//         "x":
//         {
//             "details": "test"
//         }
//     }
// }
