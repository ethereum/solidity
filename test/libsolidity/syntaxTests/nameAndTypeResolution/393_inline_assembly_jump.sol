contract C {
    function f() pure public {
        assembly {
            jump(2)
        }
    }
}
// ----
// SyntaxError: (75-82): Jump instructions and labels are low-level EVM features that can lead to incorrect stack access. Because of that they are discouraged. Please consider using "switch", "if" or "for" statements instead.
