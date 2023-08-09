contract B {
    function x() virtual external returns (uint) {
        return 1;
    }
}

contract C {
    /// @notice Hello world
    /// @dev test
    function x() virtual external returns (uint) {
        return 1;
    }
}

contract D is C, B {
    /// @inheritdoc C
    uint public override(C, B) x;
}

// ----
// ----
// :C devdoc
// {
//     "kind": "dev",
//     "methods":
//     {
//         "x()":
//         {
//             "details": "test"
//         }
//     },
//     "version": 1
// }
//
// :D devdoc
// {
//     "kind": "dev",
//     "methods": {},
//     "stateVariables":
//     {
//         "x":
//         {
//             "details": "test"
//         }
//     },
//     "version": 1
// }
