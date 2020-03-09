contract C {
    struct S { bool f; }
    S s;
    function f() internal pure returns (S storage c) {
        assembly {
            for { c_slot := s_slot } iszero(0) {} {}
        }
    }
    function g() internal pure returns (S storage c) {
        assembly {
            for { c_slot := s_slot } iszero(1) {} {}
        }
    }
}
// ----
