contract D {
    struct S { uint a; }
}

contract C is D {
    /// @inheritdoc D.S
    function f() internal {
    }
}
// ----
// DocstringParsingError 1430: (63-82): Documentation tag @inheritdoc reference "D.S" is not a contract.
