contract C {
    /// @notice example of notice
    /// @dev example of dev
    uint public state;
}
// ----
// ----
// :C devdoc
// {
//     "kind": "dev",
//     "methods": {},
//     "stateVariables":
//     {
//         "state":
//         {
//             "details": "example of dev"
//         }
//     },
//     "version": 1
// }
//
// :C userdoc
// {
//     "kind": "user",
//     "methods":
//     {
//         "state()":
//         {
//             "notice": "example of notice"
//         }
//     },
//     "version": 1
// }
