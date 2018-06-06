pragma experimental "v0.5.0";
contract C {
    function f() pure public {
        assembly {
            label:
        }
    }
}
// ----
// SyntaxError: (105-110): The use of labels is deprecated. Please use "if", "switch", "for" or function calls instead.
// SyntaxError: (105-110): Jump instructions and labels are low-level EVM features that can lead to incorrect stack access. Because of that they are discouraged. Please consider using "switch", "if" or "for" statements instead.
