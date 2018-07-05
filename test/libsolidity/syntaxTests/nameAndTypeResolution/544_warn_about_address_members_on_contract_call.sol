contract C {
    function f() view public {
        this.call;
    }
}
// ----
// TypeError: (52-61): Member "call" not found or not visible after argument-dependent lookup in contract C
