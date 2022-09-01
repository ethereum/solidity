contract C {
    enum small {A, B, C, D}
    enum larger {A, B, C, D, E}
    struct str {
        small a;
        small b;
        larger c;
        larger d;
    }
    str data;

    function test() public returns (uint256) {
        data.a = small.B;
        if (data.a != small.B) return 2;
        data.b = small.C;
        if (data.b != small.C) return 3;
        data.c = larger.D;
        if (data.c != larger.D) return 4;
        if (data.a != small.B) return 5;
        data.a = small.C;
        if (data.a != small.C) return 6;
        if (data.b != small.C) return 7;
        data.b = small.D;
        if (data.b != small.D) return 8;
        if (data.c != larger.D) return 9;
        data.c = larger.B;
        if (data.c != larger.B) return 10;
        return 1;
    }
}

// ====
// compileToEwasm: also
// ----
// test() -> 1
