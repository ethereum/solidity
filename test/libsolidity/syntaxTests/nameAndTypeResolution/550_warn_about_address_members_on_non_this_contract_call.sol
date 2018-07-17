contract C {
    function f() pure public {
        C c;
        c.call;
    }
}
// ----
// TypeError: (65-71): Member "call" not found or not visible after argument-dependent lookup in contract C. Use "address(c).call" to access this address member.
