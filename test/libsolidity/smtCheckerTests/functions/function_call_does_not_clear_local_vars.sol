contract C {
    function f() public {
        uint a = 3;
        this.f();
        assert(a == 3);
        f();
        assert(a == 3);
    }
}
// ====
// SMTEngine: all
// ----
// Warning 5740: (122-136): Unreachable code.
