pragma experimental SMTChecker;
contract C {
    function f() public {
        uint a = 3;
        this.f();
        assert(a == 3);
        f();
        assert(a == 3);
    }
}
// ----
