/// @author Lefteris
/// @title Just a test contract
contract test {
    /// @dev Mul function
    /// @author John Doe
    function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
}

// ----
// DocstringParsingError 6546: (73-119): Documentation tag @author not valid for functions.
