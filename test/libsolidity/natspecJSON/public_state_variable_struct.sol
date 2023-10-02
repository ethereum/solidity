contract Bank {
    struct Coin {
        string observeGraphicURL;
        string reverseGraphicURL;
    }

    /// @notice Get the n-th coin I own
    /// @return observeGraphicURL Front pic
    /// @return reverseGraphicURL Back pic
    Coin[] public coinStack;
}

// ----
// ----
// :Bank devdoc
// {
//     "kind": "dev",
//     "methods": {},
//     "stateVariables":
//     {
//         "coinStack":
//         {
//             "returns":
//             {
//                 "observeGraphicURL": "Front pic",
//                 "reverseGraphicURL": "Back pic"
//             }
//         }
//     },
//     "version": 1
// }
//
// :Bank userdoc
// {
//     "kind": "user",
//     "methods":
//     {
//         "coinStack(uint256)":
//         {
//             "notice": "Get the n-th coin I own"
//         }
//     },
//     "version": 1
// }
