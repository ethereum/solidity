contract C {
    modifier alwaysRevert() {
        _;
        revert();
    }
    modifier ifFlag(bool flag) {
        if (flag)
            _;
    }
    struct S { uint a; }
    S s;
    function f(bool flag) alwaysRevert() internal view {
        if (flag) s;
    }
    function g(bool flag) alwaysRevert() ifFlag(flag) internal view {
        s;
    }

}
// ----
