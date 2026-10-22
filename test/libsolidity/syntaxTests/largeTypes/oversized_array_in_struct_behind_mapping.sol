contract C {
    struct S { uint64[340282366920938463463374607431768211458][1701411834604692317316873037158841057281] x; }
    struct Outer { mapping(uint => S) m; }
    mapping(uint => S) p;
    Outer o;
    mapping(uint => S)[] arr;
}
// ----
// TypeError 1534: (170-190): Type too large for storage.
// TypeError 1534: (196-203): Type too large for storage.
// TypeError 1534: (209-233): Type too large for storage.
