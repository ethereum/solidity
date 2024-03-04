contract C {
    /// @notice C.E event
    /// @dev C.E event
    event E(uint256 value);
}

contract D {
    /// @notice D.E event
    /// @dev D.E event
    event E(uint256 value);

    function test() public {
        emit C.E(1);
        emit E(2);
    }
}

// ----
// ----
// :C devdoc
// {
//     "events":
//     {
//         "E(uint256)":
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
//         "E(uint256)":
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
//     "events":
//     {
//         "E(uint256)":
//         {
//             "details": "D.E event"
//         }
//     },
//     "kind": "dev",
//     "methods": {},
//     "version": 1
// }
//
// :D userdoc
// {
//     "events":
//     {
//         "E(uint256)":
//         {
//             "notice": "D.E event"
//         }
//     },
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
