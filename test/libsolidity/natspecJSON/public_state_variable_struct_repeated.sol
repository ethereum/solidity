contract Bank {
    struct Coin {
        string obverseGraphicURL;
        string reverseGraphicURL;
    }

    /// @notice Get the n-th coin I own
    /// @return obverseGraphicURL Front pic
    /// @return obverseGraphicURL Front pic
    Coin[] public coinStack;
}

// ----
