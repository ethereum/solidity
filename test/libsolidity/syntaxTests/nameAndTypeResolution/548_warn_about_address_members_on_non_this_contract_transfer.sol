contract C {
    function f() view public {
        C c;
        c.transfer;
    }
}
// ----
// TypeError: (65-75): Member "transfer" not found or not visible after argument-dependent lookup in contract C. Use "address(c).transfer" to access this address member.
