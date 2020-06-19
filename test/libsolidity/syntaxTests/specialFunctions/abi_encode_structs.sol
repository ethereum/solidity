contract C {
    struct S { uint x; }
    S s;
    struct T { uint y; }
    T t;
    function f() public view {
        abi.encode(s, t);
    }
    function g() public view {
        abi.encodePacked(s, t);
    }
}
// ----
// TypeError 2056: (131-132): This type cannot be encoded.
// TypeError 2056: (134-135): This type cannot be encoded.
// TypeError 9578: (200-201): Type not supported in packed mode.
// TypeError 9578: (203-204): Type not supported in packed mode.
