contract C {
    struct S {
        uint64 y;
        uint64 z;
    }
    S s;
    function f() public returns (uint256 ret) {
        assembly {
            // 2 ** 150 - 1
            sstore(s.slot, 1427247692705959881058285969449495136382746623)
        }
        s.y = 1; s.z = 2;
        delete s;
        assert(s.y == 0);
        assert(s.z == 0);
        assembly {
            ret := sload(s.slot)
        }
    }
}
// ====
// compileViaYul: true
// ----
// f() -> 0
