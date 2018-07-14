contract C {
    function f() public {
        this.callcode;
    }
}
// ----
// TypeError: (47-60): Member "callcode" not found or not visible after argument-dependent lookup in contract C
