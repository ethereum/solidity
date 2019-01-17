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
// Warning: (99-107): Assertion checker does not yet implement this type of function call.
// Warning: (141-144): Assertion checker does not support recursive function calls.
