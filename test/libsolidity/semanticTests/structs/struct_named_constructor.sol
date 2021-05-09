contract C {
    struct S {
        uint256 a;
        bool x;
    }
    S public s;

    constructor() {
        s = S({x: true, a: 1});
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// s() -> 1, true
