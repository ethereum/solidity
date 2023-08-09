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
//     }
// }
//
// :test userdoc
// {
//     "methods":
//     {
//         "state()":
//         {
//             "notice": "example of notice"
//         }
//     }
// }
