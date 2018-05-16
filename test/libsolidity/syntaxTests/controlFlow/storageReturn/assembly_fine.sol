contract C {
    struct S { bool f; }
    S s;
    function f() internal returns (S storage c) {
        assembly {
            sstore(c_slot, sload(s_slot))
        }
    }
    function g(bool flag) internal returns (S storage c) {
        // control flow in assembly will not be analyzed for now,
        // so this will not issue a warning
        assembly {
            if flag {
                sstore(c_slot, sload(s_slot))
            }
        }
    }
    function h() internal returns (S storage c) {
        // any reference from assembly will be sufficient for now,
        // so this will not issue a warning
        assembly {
            sstore(s_slot, sload(c_slot))
        }
    }
}
// ----
