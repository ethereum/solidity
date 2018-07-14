contract C {
    function f() public {
        this.transfer;
    }
}
// ----
// TypeError: (47-60): Member "transfer" not found or not visible after argument-dependent lookup in contract C
