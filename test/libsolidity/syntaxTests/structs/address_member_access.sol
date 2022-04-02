contract C {
    struct S { uint a; }
    function f() public pure returns(address) {
        S memory s = S(42);
        return s.address;
    }
}
// ----
// TypeError 9582: (129-138='s.address'): Member "address" not found or not visible after argument-dependent lookup in struct C.S memory.
