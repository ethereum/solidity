contract C {
    struct S {
        uint32 a;
        uint32[3] b;
        uint32[] x;
    }
    S s;
    function f() public returns (uint256 ret) {
        assembly {
            // 2 ** 150 - 1
            sstore(s.slot, 1427247692705959881058285969449495136382746623)
        }
        s.a = 1;
        s.b[0] = 2; s.b[1] = 3;
        s.x.push(4); s.x.push(5);
        delete s;
        assert(s.a == 0);
        assert(s.b[0] == 0);
        assert(s.b[1] == 0);
        assert(s.x.length == 0);
        assembly {
            ret := sload(s.slot)
        }
    }
}
// ====
// compileViaYul: true
// ----
// f() -> 0
// gas irOptimized: 111896
