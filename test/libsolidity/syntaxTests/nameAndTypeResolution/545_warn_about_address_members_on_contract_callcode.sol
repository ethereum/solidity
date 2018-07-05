contract C {
    function f() view public {
        this.callcode;
    }
}
// ----
// TypeError: (52-65): Member "callcode" not found or not visible after argument-dependent lookup in contract C
