contract X {
    /// @notice Userdoc for event E.
    /// @dev Devdoc for event E.
    event E();
}

contract C {
    function g() public {
        emit X.E();
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
//             "details": "Devdoc for event E."
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
//             "notice": "Userdoc for event E."
//         }
//     },
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
//
// :X devdoc
// {
//     "events":
//     {
//         "E()":
//         {
//             "details": "Devdoc for event E."
//         }
//     },
//     "kind": "dev",
//     "methods": {},
//     "version": 1
// }
//
// :X userdoc
// {
//     "events":
//     {
//         "E()":
//         {
//             "notice": "Userdoc for event E."
//         }
//     },
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
