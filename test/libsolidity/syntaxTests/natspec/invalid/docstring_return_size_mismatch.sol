abstract contract C {
    /// @param id Some identifier
    /// @return No value returned
    function vote(uint id) public virtual returns (uint value);

    /// @return No value returned
    function unvote(uint id) public virtual returns (uint value);
}
// ----
// DocstringParsingError: (26-89): Documentation tag "@return No value returned" does not contain the name of its return parameter.
// DocstringParsingError: (159-188): Documentation tag "@return No value returned" does not contain the name of its return parameter.
