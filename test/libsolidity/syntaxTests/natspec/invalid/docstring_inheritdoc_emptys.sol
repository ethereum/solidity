contract C {
    /// @inheritdoc
    function f() internal {
    }
    /// @inheritdoc .
    function f() internal {
    }
    /// @inheritdoc C..f
    function f() internal {
    }
    /// @inheritdoc C.
    function f() internal {
    }
}
// ----
// DocstringParsingError 1933: (17-32): Expected contract name following documentation tag @inheritdoc.
// DocstringParsingError 5967: (71-88): Documentation tag @inheritdoc reference "." is malformed.
// DocstringParsingError 5967: (127-147): Documentation tag @inheritdoc reference "C..f" is malformed.
// DocstringParsingError 5967: (186-204): Documentation tag @inheritdoc reference "C." is malformed.
