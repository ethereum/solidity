pragma experimental SMTChecker;
contract C
{
    function f() public view {
        assert(c > 0);
    }
    uint c;
}
// ----
// Warning: (84-97): Assertion violation happens here
