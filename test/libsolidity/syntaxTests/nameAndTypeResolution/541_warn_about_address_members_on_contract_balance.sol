contract C {
    function f() view public {
        this.balance;
    }
}
// ----
// TypeError: (52-64): Member "balance" not found or not visible after argument-dependent lookup in contract C
