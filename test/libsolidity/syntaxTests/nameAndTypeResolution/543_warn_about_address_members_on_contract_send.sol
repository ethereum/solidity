contract C {
    function f() view public {
        this.send;
    }
}
// ----
// TypeError: (52-61): Member "send" not found or not visible after argument-dependent lookup in contract C
