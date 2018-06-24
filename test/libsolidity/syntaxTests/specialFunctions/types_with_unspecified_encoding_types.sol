contract C {
    struct S { uint x; }
    S s;
    struct T { uint y; }
    T t;
    enum A { X, Y }
    function f() public pure {
        bool a = address(this).delegatecall(S, A, A.X, T, uint, uint[]);
    }
}
// ----
// TypeError: (176-177): This type cannot be encoded.
// TypeError: (179-180): This type cannot be encoded.
// TypeError: (187-188): This type cannot be encoded.
// TypeError: (190-194): This type cannot be encoded.
// TypeError: (196-202): This type cannot be encoded.
