abstract contract C {
    /// @param id Some identifier
    /// @return No value returned
    function vote(uint id) public virtual returns (uint value);
}
// ----
// DocstringParsingError 5856: (26-89): Documentation tag "@return No value returned" does not contain the name of its return parameter.
