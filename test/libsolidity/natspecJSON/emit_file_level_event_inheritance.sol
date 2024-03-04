/// @notice Userdoc for file-level event E.
/// @dev Devdoc for file-level E.
event E();

contract C {
    function f() public {
        emit E();
    }
}

contract D is C {}

// ----
// ----
// :C devdoc
// {
//     "events":
//     {
//         "E()":
//         {
//             "details": "Devdoc for file-level E."
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
//             "notice": "Userdoc for file-level event E."
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
//         "E()":
//         {
//             "details": "Devdoc for file-level E."
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
//         "E()":
//         {
//             "notice": "Userdoc for file-level event E."
//         }
//     },
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
