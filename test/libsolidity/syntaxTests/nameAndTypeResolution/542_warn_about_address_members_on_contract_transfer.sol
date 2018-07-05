contract C {
    function f() view public {
        this.transfer;
    }
}
// ----
// TypeError: (52-65): Member "transfer" not found or not visible after argument-dependent lookup in contract C
