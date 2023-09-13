contract test {
    /// @dev Multiplies a number by 7 and adds second parameter
    /// @param a Documentation for the first parameter
    /// @param
    function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
}

// ----
// DocstringParsingError 3335: (20-149): No param name given
