contract C {
    struct S { bool f; }
    S s;
    function f() internal pure returns (S storage c) {
        assembly {
            for { c.slot := s.slot } iszero(0) {} {}
        }
    }
    function g() internal pure returns (S storage c) {
        assembly {
            for { c.slot := s.slot } iszero(1) {} {}
        }
    }
}
// ----
