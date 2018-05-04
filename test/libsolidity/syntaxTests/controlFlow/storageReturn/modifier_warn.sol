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
// Warning: (249-250): This variable is of storage pointer type and might be returned without assignment. This can cause storage corruption. Assign the variable (potentially from itself) to remove this warning.
// Warning: (367-368): This variable is of storage pointer type and might be returned without assignment. This can cause storage corruption. Assign the variable (potentially from itself) to remove this warning.
