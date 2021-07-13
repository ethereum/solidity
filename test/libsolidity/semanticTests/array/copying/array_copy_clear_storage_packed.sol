contract C {
    uint128[] x;
    uint64[] x1;
    uint120[] x2;
    function f() public returns(uint128) {
        x.push(42); x.push(42); x.push(42); x.push(42);
        uint128[] memory y = new uint128[](1);
        y[0] = 23;
        x = y;
        assembly { sstore(x.slot, 4) }
        assert(x[0] == 23);
        assert(x[2] == 0);
        assert(x[3] == 0);
        return x[1];
    }

    function g() public returns(uint64) {
        x1.push(42); x1.push(42); x1.push(42); x1.push(42);
        uint64[] memory y = new uint64[](1);
        y[0] = 23;
        x1 = y;
        assembly { sstore(x1.slot, 4) }
        assert(x1[0] == 23);
        assert(x1[2] == 0);
        assert(x1[3] == 0);
        return x1[1];
    }

    function h() public returns(uint120) {
        x2.push(42); x2.push(42); x2.push(42); x2.push(42);
        uint120[] memory y = new uint120[](1);
        y[0] = 23;
        x2 = y;
        assembly { sstore(x2.slot, 4) }
        assert(x2[0] == 23);
        assert(x2[2] == 0);
        assert(x2[3] == 0);
        return x2[1];
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0
// gas irOptimized: 92773
// gas legacy: 93006
// gas legacyOptimized: 92261
// g() -> 0
// h() -> 0
// gas irOptimized: 92838
// gas legacy: 93028
// gas legacyOptimized: 92303
