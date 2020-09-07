contract C {
    struct S {
        uint256 a;
        bool x;
    }
    S public s;

    constructor() {
        s = S({a: 1, x: true});
    }
}

// ====
// compileViaYul: also
// ----
// s() -> 1, true
