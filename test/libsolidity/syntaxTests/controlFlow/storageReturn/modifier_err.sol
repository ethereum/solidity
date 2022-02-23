contract C {
    modifier callAndRevert() {
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

    function g(bool flag) ifFlag(flag) callAndRevert() internal view returns(S storage) {
        return s;
    }
}
// ----
// TypeError 3464: (246-255): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (361-370): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
