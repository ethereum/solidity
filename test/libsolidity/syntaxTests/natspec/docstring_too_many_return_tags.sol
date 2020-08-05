abstract contract C {
    /// @param id Some identifier
    /// @return value Some value
	/// @return value2 Some value 2
    function vote(uint id) public virtual returns (uint value);
}
// ----
// DocstringParsingError 2604: (26-121): Documentation tag "@return value2 Some value 2" exceeds the number of return parameters.
