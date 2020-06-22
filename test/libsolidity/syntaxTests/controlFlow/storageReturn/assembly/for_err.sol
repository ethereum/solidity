contract C {
    struct S { bool f; }
    S s;
    function f() internal pure returns (S storage c) {
        assembly {
            for {} eq(0,0) { c_slot := s_slot } {}
        }
    }
    function g() internal pure returns (S storage c) {
        assembly {
            for {} eq(0,1) { c_slot := s_slot } {}
        }
    }
    function h() internal pure returns (S storage c) {
        assembly {
            for {} eq(0,0) {} { c_slot := s_slot }
        }
    }
    function i() internal pure returns (S storage c) {
        assembly {
            for {} eq(0,1) {} { c_slot := s_slot }
        }
    }
}
// ----
// TypeError 3464: (87-98): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (228-239): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (369-380): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (510-521): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
