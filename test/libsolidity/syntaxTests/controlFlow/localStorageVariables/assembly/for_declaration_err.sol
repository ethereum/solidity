contract C {
    struct S { bool f; }
    S s;
    function f() internal pure {
        S storage c;
        assembly {
            for {} eq(0,0) { c_slot := s_slot } {}
        }
        c;
    }
    function g() internal pure {
        S storage c;
        assembly {
            for {} eq(0,1) { c_slot := s_slot } {}
        }
        c;
    }
    function h() internal pure {
        S storage c;
        assembly {
            for {} eq(0,0) {} { c_slot := s_slot }
        }
        c;
    }
    function i() internal pure {
        S storage c;
        assembly {
            for {} eq(0,1) {} { c_slot := s_slot }
        }
        c;
    }
}
// ----
// TypeError: (189-190): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError: (340-341): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError: (491-492): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError: (642-643): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.

