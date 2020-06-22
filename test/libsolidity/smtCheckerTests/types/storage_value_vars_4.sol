pragma experimental SMTChecker;
contract C
{
    function f() public view {
        assert(c > 0);
    }
    uint c;
}
// ----
// Warning 4661: (84-97): Assertion violation happens here
