pragma experimental SMTChecker;

contract C {
    uint a;
    bool b;

    function f() public returns(uint c) {
        c = b ? a + 1 : a--;
        assert(c > a);
    }
}
// ----
