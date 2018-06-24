contract C {
    struct S { uint a; }
    modifier m(S storage x) {
        x;
        _;
    }
}
