contract C {
    modifier revertIfNoReturn() {
        _;
        revert();
    }
    modifier ifFlag(bool flag) {
        if (flag)
            _;
    }
    struct S { uint a; }
    S s;
    function f(bool flag) ifFlag(flag) internal view returns(S storage) {
        return s;
    }

    function g(bool flag) ifFlag(flag) revertIfNoReturn() internal view returns(S storage) {
        return s;
    }
}
// ----
// TypeError: (249-258): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// TypeError: (367-376): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
