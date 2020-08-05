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

// ----
// s() -> 1, true
