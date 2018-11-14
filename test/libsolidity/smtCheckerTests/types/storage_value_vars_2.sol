pragma experimental SMTChecker;
contract C
{
    address a;
    bool b;
    uint c;
    function f() public view {
        assert(c > 0);
    }
}
// ----
// Warning: (123-136): Assertion violation happens here
