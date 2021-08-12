contract C {
    struct S {
        uint32 a;
        S[] x;
    }
    S s;
    function f() public returns (uint256 r1, uint256 r2, uint256 r3) {
        assembly {
            // 2 ** 150 - 1
            sstore(s.slot, 1427247692705959881058285969449495136382746623)
        }
        s.a = 1;
        s.x.push(); s.x.push();
        S storage ptr1 = s.x[0];
        S storage ptr2 = s.x[1];
        assembly {
            // 2 ** 150 - 1
            sstore(ptr1.slot, 1427247692705959881058285969449495136382746623)
            sstore(ptr2.slot, 1427247692705959881058285969449495136382746623)
        }
        s.x[0].a = 2; s.x[1].a = 3;
        delete s;
        assert(s.a == 0);
        assert(s.x.length == 0);
        assembly {
            r1 := sload(s.slot)
            r2 := sload(ptr1.slot)
            r3 := sload(ptr2.slot)
        }
    }
}
// ====
// compileViaYul: true
// ----
// f() -> 0, 0, 0
// gas irOptimized: 117505
