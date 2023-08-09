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
// :C userdoc
// {
//     "kind": "user",
//     "methods":
//     {
//         "x()":
//         {
//             "notice": "Hello world"
//         }
//     },
//     "version": 1
// }
//
// :D userdoc
// {
//     "kind": "user",
//     "methods":
//     {
//         "x()":
//         {
//             "notice": "Hello world"
//         }
//     },
//     "version": 1
// }
