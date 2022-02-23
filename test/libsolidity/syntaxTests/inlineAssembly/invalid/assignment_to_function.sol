contract C {
    function f() public pure {
        assembly {
            function f() {}
            f := 1
        }
    }
}
// ----
// TypeError 2657: (103-104): Assignment requires variable.
