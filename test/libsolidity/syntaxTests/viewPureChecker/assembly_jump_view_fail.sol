contract C {
    function k() public view {
        assembly { jump(2) }
    }
}
// ----
// SyntaxError: (63-70): Jump instructions and labels are low-level EVM features that can lead to incorrect stack access. Because of that they are discouraged. Please consider using "switch", "if" or "for" statements instead.
