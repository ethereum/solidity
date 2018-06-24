contract C {
    function k() public {
        assembly { jump(2) }
    }
}
// ----
// Warning: (58-65): Jump instructions and labels are low-level EVM features that can lead to incorrect stack access. Because of that they are discouraged. Please consider using "switch", "if" or "for" statements instead.
