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
// TypeError: (131-132): This type cannot be encoded.
// TypeError: (134-135): This type cannot be encoded.
// TypeError: (200-201): Type not supported in packed mode.
// TypeError: (203-204): Type not supported in packed mode.
