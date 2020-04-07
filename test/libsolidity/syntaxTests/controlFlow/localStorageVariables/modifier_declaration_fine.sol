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
    function f(bool flag) revertIfNoReturn() internal view {
        if (flag) s;
    }
    function g(bool flag) revertIfNoReturn() ifFlag(flag) internal view {
        s;
    }

}
// ----
