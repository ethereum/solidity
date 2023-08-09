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
//     }
// }
//
// :Bank userdoc
// {
//     "methods":
//     {
//         "coinStack(uint256)":
//         {
//             "notice": "Get the n-th coin I own"
//         }
//     }
// }
