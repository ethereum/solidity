contract C {
    struct S { bool f; }
    S s;
    function f() internal view returns (S storage c) {
        false && (c = s).f;
    }
    function g() internal view returns (S storage c) {
        true || (c = s).f;
    }
    function h() internal view returns (S storage c) {
        // expect error, although this is always fine
        true && (false || (c = s).f);
    }
}
// ----
// TypeError: (87-98): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// TypeError: (176-187): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// TypeError: (264-275): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
