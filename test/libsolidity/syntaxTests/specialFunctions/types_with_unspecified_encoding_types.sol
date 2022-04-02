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
// TypeError 2056: (174-175='S'): This type cannot be encoded.
// TypeError 2056: (177-178='A'): This type cannot be encoded.
// TypeError 2056: (185-186='T'): This type cannot be encoded.
// TypeError 2056: (188-192='uint'): This type cannot be encoded.
// TypeError 2056: (194-200='uint[]'): This type cannot be encoded.
