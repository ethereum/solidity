contract C {
    struct S { bool f; }
    S s;
    function f(bool flag) internal pure {
        S storage c;
        assembly {
            switch flag
            case 0 { c_slot := s_slot }
            default { c_slot := s_slot }
        }
        c;
    }
    function g(uint256 a) internal pure {
        S storage c;
        assembly {
            switch a
            case 0 { revert(0, 0) }
            default { c_slot := s_slot }
        }
        c;
    }
}
// ----
