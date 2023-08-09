contract test {
    /// @notice example of notice
    /// @dev example of dev
    /// @return returns state
    uint public state;
}

// ----
// ----
// :test devdoc
// {
//     "kind": "dev",
//     "methods": {},
//     "stateVariables":
//     {
//         "state":
//         {
//             "details": "example of dev",
//             "return": "returns state",
//             "returns":
//             {
//                 "_0": "returns state"
//             }
//         }
//     },
//     "version": 1
// }
//
// :test userdoc
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
