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
// TypeError: (200-201): This type cannot be encoded.
// TypeError: (203-204): This type cannot be encoded.
