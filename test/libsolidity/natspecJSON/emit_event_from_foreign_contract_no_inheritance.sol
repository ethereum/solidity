// Tests that emitting an event from contract C in contract D does not inherit natspec from C.E

contract C {
    /// @notice C.E event
    /// @dev C.E event
    event E();
}

contract D {
    event E();

    function test() public {
        emit C.E();
    }
}

// ----
// ----
// :C devdoc
// {
//     "events":
//     {
//         "E()":
//         {
//             "details": "C.E event"
//         }
//     },
//     "kind": "dev",
//     "methods": {},
//     "version": 1
// }
//
// :C userdoc
// {
//     "events":
//     {
//         "E()":
//         {
//             "notice": "C.E event"
//         }
//     },
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
//
// :D devdoc
// {
//     "kind": "dev",
//     "methods": {},
//     "version": 1
// }
//
// :D userdoc
// {
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
