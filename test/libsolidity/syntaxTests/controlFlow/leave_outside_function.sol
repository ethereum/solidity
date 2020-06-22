contract C {
    function f() public pure {
        assembly {
            // Make sure this doesn't trigger the unimplemented assertion in the control flow builder.
            leave
        }
    }
}
// ----
// SyntaxError 8149: (178-183): Keyword "leave" can only be used inside a function.
