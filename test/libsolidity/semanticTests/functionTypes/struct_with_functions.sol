struct S {
    uint16 a;
    function() returns (uint) x;
    uint16 b;
}
contract Flow {
    S[2] t;

    function X() internal pure returns (uint) {
        return 1;
    }

    function Y() internal pure returns (uint) {
        return 2;
    }

    constructor() {
        t[0].a = 0xff07;
        t[0].b = 0xff07;
        t[1].x = Y;
        t[1].a = 0xff07;
        t[1].b = 0xff07;
        t[0].x = X;
    }

    function f() public returns (uint, uint) {
        return (t[0].x(), t[1].x());
    }
}

// ====
// compileToEwasm: also
// ----
// f() -> 1, 2
