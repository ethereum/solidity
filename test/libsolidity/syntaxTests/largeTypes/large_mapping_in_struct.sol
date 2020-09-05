contract C {
    struct S {
        mapping (uint => uint[2][2**255]) A;
    }
    S s;
}
// ----
// TypeError 1534: (83-86): Type too large for storage.
