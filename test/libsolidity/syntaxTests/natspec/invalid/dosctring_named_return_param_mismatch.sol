abstract contract C {
    /// @param id Some identifier
    /// @return No value returned
    function vote(uint id) public virtual returns (uint value);
}
// ----
// DocstringParsingError: Doc tag "@return No value returned" does not contain the name of its return parameter.