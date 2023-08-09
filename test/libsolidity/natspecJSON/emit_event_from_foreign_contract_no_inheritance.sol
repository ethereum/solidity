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
