contract D {
}

contract C is D {
    /// @inheritdoc D
    function f() internal {
    }
}
// ----
// DocstringParsingError 4682: (38-55): Documentation tag @inheritdoc references contract "D", but the contract does not contain a function that is overridden by this function.
