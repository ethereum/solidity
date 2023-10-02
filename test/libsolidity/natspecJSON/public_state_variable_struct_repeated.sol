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
// DocstringParsingError 5856: (113-236): Documentation tag "@return obverseGraphicURL Front pic" does not contain the name of its return parameter.
