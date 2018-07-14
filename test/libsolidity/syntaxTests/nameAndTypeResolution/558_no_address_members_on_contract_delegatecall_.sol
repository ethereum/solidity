contract C {
    function f() public {
        this.delegatecall;
    }
}
// ----
// TypeError: (47-64): Member "delegatecall" not found or not visible after argument-dependent lookup in contract C
