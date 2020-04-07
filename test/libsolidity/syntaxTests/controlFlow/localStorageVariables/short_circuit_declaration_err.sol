contract C {
    struct S { bool f; }
    S s;
    function f() internal view {
        S storage c;
        false && (c = s).f;
        c;
    }
    function g() internal view {
        S storage c;
        true || (c = s).f;
        c;
    }
    function h() internal view {
        S storage c;
        // expect error, although this is always fine
        true && (false || (c = s).f);
        c;
    }
}
// ----
// TypeError: (137-138): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
//  TypeError: (235-236): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
//  TypeError: (398-399): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
