contract C {
    struct S {
        uint256 x;
        uint128 y;
        uint32 z;
        uint128[3] a1;
        uint128[] a2;
    }
    uint8 b = 23;
    S[] s;
    uint8 a = 17;
    function f() public {
        s.push();
        assert(s[0].x == 0);
        assert(s[0].y == 0);
        assert(s[0].z == 0);
        assert(s[0].a1[0] == 0);
        assert(s[0].a1[1] == 0);
        assert(s[0].a1[2] == 0);
        assert(s[0].a2.length == 0);
        assert(b == 23);
        assert(a == 17);
    }
}
// ====
// compileViaYul: also
// ----
// f() ->
