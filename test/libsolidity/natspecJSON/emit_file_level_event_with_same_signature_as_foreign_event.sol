/// @notice Userdoc for file-level event E.
/// @dev Devdoc for file-level E.
event E();

contract F {
    /// @notice Userdoc for event F.E.
    /// @dev Devdoc for event F.E.
    event E();
}

contract C {
    function f() public {
        emit E();
        emit F.E();
    }
}

// ----
// ----
// :C devdoc
// {
//     "kind": "dev",
//     "methods": {},
//     "version": 1
// }
//
// :C userdoc
// {
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
//
// :F devdoc
// {
//     "events":
//     {
//         "E()":
//         {
//             "details": "Devdoc for event F.E."
//         }
//     },
//     "kind": "dev",
//     "methods": {},
//     "version": 1
// }
//
// :F userdoc
// {
//     "events":
//     {
//         "E()":
//         {
//             "notice": "Userdoc for event F.E."
//         }
//     },
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
