contract C {
    struct S { bool f; }
    S s;
    function f(uint256 a) internal pure returns (S storage c) {
        assembly {
            switch a
            case 0 { c.slot := s.slot }
        }
    }
    function g(bool flag) internal pure returns (S storage c) {
        assembly {
            switch flag
            case 0 { c.slot := s.slot }
            case 1 { c.slot := s.slot }
        }
    }
    function h(uint256 a) internal pure returns (S storage c) {
        assembly {
            switch a
            case 0 { c.slot := s.slot }
            default { return(0,0) }
        }
    }
}
// ----
// TypeError 3464: (96-107): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (256-267): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
