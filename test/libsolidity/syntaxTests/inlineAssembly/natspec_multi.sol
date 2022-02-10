function f() pure {
    /// @unrelated bogus-value

    /// @before bogus-value
    ///
    /// @solidity a   memory-safe-assembly b    c
    ///           d
    /// @after bogus-value
    assembly {}
    /// @solidity memory-safe-assembly a a a
    ///           memory-safe-assembly
    assembly {}
}
// ----
// Warning 6269: (189-200): Unexpected NatSpec tag "after" with value "bogus-value" in inline assembly.
// Warning 6269: (189-200): Unexpected NatSpec tag "before" with value "bogus-value" in inline assembly.
// Warning 8787: (189-200): Unexpected value for @solidity tag in inline assembly: a
// Warning 8787: (189-200): Unexpected value for @solidity tag in inline assembly: b
// Warning 8787: (189-200): Unexpected value for @solidity tag in inline assembly: c
// Warning 8787: (189-200): Unexpected value for @solidity tag in inline assembly: d
// Warning 8787: (289-300): Unexpected value for @solidity tag in inline assembly: a
// Warning 4377: (289-300): Value for @solidity tag in inline assembly specified multiple times: a
// Warning 4377: (289-300): Value for @solidity tag in inline assembly specified multiple times: memory-safe-assembly
