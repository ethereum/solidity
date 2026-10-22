contract C {
    struct RecursiveStruct { RecursiveStruct[] vals; }
    struct A { B[] bs; }
    struct B { mapping(uint => A) as_; }
    RecursiveStruct[] store;
    A[2] indirect;
    function f() private pure { RecursiveStruct[1] memory val; val; }
}
// ----
