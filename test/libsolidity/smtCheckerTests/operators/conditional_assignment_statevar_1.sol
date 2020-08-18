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
// Warning 2661: (129-134): Overflow (resulting value larger than 2**256 - 1) happens here
