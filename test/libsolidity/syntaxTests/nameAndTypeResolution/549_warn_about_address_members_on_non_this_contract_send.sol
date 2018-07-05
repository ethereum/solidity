contract C {
    function f() view public {
        C c;
        c.send;
    }
}
// ----
// TypeError: (65-71): Member "send" not found or not visible after argument-dependent lookup in contract C
