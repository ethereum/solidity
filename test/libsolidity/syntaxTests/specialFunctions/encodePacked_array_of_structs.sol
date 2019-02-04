contract C {
    struct S { uint x; }
    function f() public pure {
        S[] memory s;
        abi.encodePacked(s);
    }
}
// ----
// TypeError: (116-117): Type not supported in packed mode.
