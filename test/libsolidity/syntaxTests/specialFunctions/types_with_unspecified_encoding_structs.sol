contract C {
    struct S { uint x; }
    S s;
    struct T { uint y; }
    T t;
    function f() public view {
        bytes32 a = sha256(abi.encodePacked(s, t));
        a;
    }
}
// ----
// TypeError 9578: (156-157='s'): Type not supported in packed mode.
// TypeError 9578: (159-160='t'): Type not supported in packed mode.
