contract C {
    struct S { bool f; }
    S s;
    function f(bool flag) internal pure {
        S storage c;
        assembly {
            if flag { c.slot := s.slot }
        }
        c;
    }
}
// ----
// TypeError 3464: (188-189): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
