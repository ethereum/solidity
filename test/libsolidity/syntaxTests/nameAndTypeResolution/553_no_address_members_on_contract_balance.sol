contract C {
    function f() public {
        this.balance;
    }
}
// ----
// TypeError: (47-59): Member "balance" not found or not visible after argument-dependent lookup in contract C
