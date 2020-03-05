contract C {
    struct S { bool f; }
    S s;
    function f(bool flag) internal pure returns (S storage c) {
        assembly {
            if flag { c_slot := s_slot }
        }
    }
}
// ----
// TypeError: (96-107): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
