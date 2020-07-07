contract C {
    struct S { uint x; }
    S s;
    struct T { uint y; }
    T t;
    enum A { X, Y }
    function f() public pure {
        bytes memory a = abi.encodePacked(S, A, A.X, T, uint, uint[]);
        a;
    }
}
// ----
// TypeError 2056: (174-175): This type cannot be encoded.
// TypeError 2056: (177-178): This type cannot be encoded.
// TypeError 2056: (185-186): This type cannot be encoded.
// TypeError 2056: (188-192): This type cannot be encoded.
// TypeError 2056: (194-200): This type cannot be encoded.
