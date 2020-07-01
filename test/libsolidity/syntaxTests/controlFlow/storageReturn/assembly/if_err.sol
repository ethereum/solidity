contract C {
    struct S { bool f; }
    S s;
    function f(bool flag) internal pure returns (S storage c) {
        assembly {
            if flag { c.slot := s.slot }
        }
    }
}
// ----
// TypeError 3464: (96-107): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
