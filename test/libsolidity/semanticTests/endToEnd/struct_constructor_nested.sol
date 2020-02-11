contract C {
    struct X {
        uint x1;
        uint x2;
    }
    struct S {
        uint s1;
        uint[3] s2;
        X s3;
    }
    S s;
    constructor() public {
        uint[3] memory s2;
        s2[1] = 9;
        s = S(1, s2, X(4, 5));
    }

    function get() public returns(uint s1, uint[3] memory s2, uint x1, uint x2) {
        s1 = s.s1;
        s2 = s.s2;
        x1 = s.s3.x1;
        x2 = s.s3.x2;
    }
}

// ----
// get() -> 1, 0, 9, 0, 4, 5
