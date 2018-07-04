contract C {
    function k() public view {
        assembly { jump(2) }
    }
}
// ----
// Warning: (63-70): Jump instructions and labels are low-level EVM features that can lead to incorrect stack access. Because of that they are discouraged. Please consider using "switch", "if" or "for" statements instead.
// TypeError: (63-70): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
