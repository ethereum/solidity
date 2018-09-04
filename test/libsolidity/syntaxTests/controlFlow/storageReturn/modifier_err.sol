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
// TypeError: (249-258): This variable is of storage pointer type and might be returned without assignment and could be used uninitialized. Assign the variable (potentially from itself) to fix this error.
// TypeError: (367-376): This variable is of storage pointer type and might be returned without assignment and could be used uninitialized. Assign the variable (potentially from itself) to fix this error.
