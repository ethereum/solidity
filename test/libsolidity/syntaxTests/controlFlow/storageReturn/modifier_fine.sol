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
    function f(bool flag) revertIfNoReturn() internal view returns(S storage) {
        if (flag) return s;
    }
    function g(bool flag) revertIfNoReturn() ifFlag(flag) internal view returns(S storage) {
        return s;
    }

}
// ----
