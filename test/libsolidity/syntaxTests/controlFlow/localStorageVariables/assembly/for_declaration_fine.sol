contract C {
    struct S { bool f; }
    S s;
    function f() internal pure {
        S storage c;
        assembly {
            for { c.slot := s.slot } iszero(0) {} {}
        }
        c;
    }
    function g() internal pure {
        S storage c;
        assembly {
            for { c.slot := s.slot } iszero(1) {} {}
        }
        c;
    }
}
// ----
