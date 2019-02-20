contract C {
    function f() pure public {
        assembly {
            label:
        }
    }
}
// ----
// SyntaxError: (75-80): The use of labels is disallowed. Please use "if", "switch", "for" or function calls instead.
// SyntaxError: (75-80): Jump instructions and labels are low-level EVM features that can lead to incorrect stack access. Because of that they are discouraged. Please consider using "switch", "if" or "for" statements instead.
