function f() pure {
    /// @unrelated bogus-value

    /// @before
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
// Warning 6269: (177-188): Unexpected NatSpec tag "after" with value "bogus-value" in inline assembly.
// Warning 6269: (177-188): Unexpected NatSpec tag "before" with value "@solidity a   memory-safe-assembly b    c           d" in inline assembly.
// Warning 8787: (277-288): Unexpected value for @solidity tag in inline assembly: a
// Warning 4377: (277-288): Value for @solidity tag in inline assembly specified multiple times: a
// Warning 4377: (277-288): Value for @solidity tag in inline assembly specified multiple times: memory-safe-assembly
