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
// Warning: (99-103): Assertion checker does not yet support the type of this variable.
// Warning: (141-144): Assertion checker does not support recursive function calls.
