contract C {
    function f() view public {
        C c;
        c.transfer;
    }
}
// ----
// TypeError 3125: (65-75='c.transfer'): Member "transfer" not found or not visible after argument-dependent lookup in contract C. Use "address(c).transfer" to access this address member.
