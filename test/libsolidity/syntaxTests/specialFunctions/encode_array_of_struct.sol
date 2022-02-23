pragma abicoder               v2;
contract C {
    struct S { uint x; }
    function f() public pure {
        S[] memory s;
        abi.encode(s);
    }
}
// ----
