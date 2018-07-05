contract C {
    function f() pure public {
        C c;
        c.delegatecall;
    }
}
// ----
// TypeError: (65-79): Member "delegatecall" not found or not visible after argument-dependent lookup in contract C
