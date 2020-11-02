pragma experimental SMTChecker;

contract C {
    function f() public pure {
        assert(msg.sig == this.f.selector);
    }
}
// ----
