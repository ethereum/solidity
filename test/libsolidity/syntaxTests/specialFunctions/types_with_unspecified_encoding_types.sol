contract C {
    struct S { uint x; }
    S s;
    struct T { }
    T t;
    enum A { X, Y }
    function f() public pure {
        bool a = address(this).delegatecall(S, A, A.X, T, uint, uint[]);
    }
}
// ----
// Warning: Defining empty structs is deprecated.
// TypeError: This type cannot be encoded.
// TypeError: This type cannot be encoded.
// TypeError: This type cannot be encoded.
// TypeError: This type cannot be encoded.
// TypeError: This type cannot be encoded.
