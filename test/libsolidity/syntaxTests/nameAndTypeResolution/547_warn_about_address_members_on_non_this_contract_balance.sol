contract C {
    function f() view public {
        C c;
        c.balance;
    }
}
// ----
// TypeError: (65-74): Member "balance" not found or not visible after argument-dependent lookup in contract C
