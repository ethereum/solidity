contract test {
    /// @dev Multiplies a number by 7 and adds second parameter
    /// @param a Documentation for the first parameter
    /// @param se
    function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
}

// ----
// DocstringParsingError 9942: (20-152): No description given for param se
