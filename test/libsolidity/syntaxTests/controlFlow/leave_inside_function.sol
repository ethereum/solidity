contract C {
    function f() public pure {
        assembly {
            function f() {
                // Make sure this doesn't trigger the unimplemented assertion in the control flow builder.
                leave
            }
        }
    }
}
// ----
