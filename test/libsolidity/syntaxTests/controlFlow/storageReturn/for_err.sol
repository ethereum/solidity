contract C {
    struct S { bool f; }
    S s;
    function f() internal view returns (S storage c) {
        for(;; c = s) {
        }
    }
    function g() internal view returns (S storage c) {
        for(;;) {
            c = s;
        }
    }
}
// ----
// TypeError 3464: (87-98='S storage c'): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (182-193='S storage c'): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
