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
    function f(bool flag) callAndRevert() internal view returns(S storage) {
        if (flag) return s;
    }
    function g(bool flag) callAndRevert() ifFlag(flag) internal view returns(S storage) {
        return s;
    }

}
// ----
