contract C {
    function f() pure public {
        C c;
        c.callcode;
    }
}
// ----
// TypeError: (65-75): Member "callcode" not found or not visible after argument-dependent lookup in contract C. Use "address(c).callcode" to access this address member.
