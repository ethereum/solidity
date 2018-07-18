contract C {
    function f() public returns (C) { return this; }
    function g() public returns (uint) { return f().balance(); }
}
// ----
// TypeError: (114-125): Member "balance" not found or not visible after argument-dependent lookup in contract C. Use "address(...).balance" to access this address member.
