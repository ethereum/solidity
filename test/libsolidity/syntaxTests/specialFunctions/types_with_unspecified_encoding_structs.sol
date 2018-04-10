contract C {
    struct S { uint x; }
    S s;
    struct T { }
    T t;
    function f() public pure {
        bytes32 a = sha256(s, t);
        a;
    }
}
// ----
// Warning: (51-63): Defining empty structs is deprecated.
// TypeError: (131-132): This type cannot be encoded.
// TypeError: (134-135): This type cannot be encoded.
