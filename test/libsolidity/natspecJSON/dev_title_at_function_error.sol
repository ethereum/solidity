/// @author Lefteris
/// @title Just a test contract
contract test {
    /// @dev Mul function
    /// @title I really should not be here
    function mul(uint a, uint second) public returns (uint d) { return a * 7 + second; }
}

// ----
// DocstringParsingError 6546: (73-137): Documentation tag @title not valid for functions.
