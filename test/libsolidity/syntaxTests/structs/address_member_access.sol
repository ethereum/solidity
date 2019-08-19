contract C {
    struct S { uint a; }
    function f() public pure returns(address) {
        S memory s = S(42);
        return s.address;
    }
}
// ----
// TypeError: (129-138): Member "address" not found or not visible after argument-dependent lookup in struct C.S memory.
