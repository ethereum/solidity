contract test {
    /// @param a the parameter a is really nice and very useful
    /// @param second the second parameter is not very useful, it just provides additional confusion
    /// @return return should not work within constructors
    constructor(uint a, uint second) { }
}

// ----
// DocstringParsingError 6546: (20-239): Documentation tag @return not valid for constructor.
// DocstringParsingError 2604: (20-239): Documentation tag "@return return should not work within constructors" exceeds the number of return parameters.
