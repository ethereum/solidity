contract C {
    struct S { uint x; }
    function f() public pure {
        S[] memory s;
        abi.encodePacked(s);
    }
}
// ----
// TypeError 9578: (116-117='s'): Type not supported in packed mode.
