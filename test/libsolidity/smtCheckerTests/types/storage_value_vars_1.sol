pragma experimental SMTChecker;
contract C
{
    address a;
    bool b;
    uint c;
    function f(uint x) public {
        if (x == 0)
        {
            a = 0x0000000000000000000000000000000000000100;
            b = true;
        }
        else
        {
            a = 0x0000000000000000000000000000000000000200;
            b = false;
        }
        assert(a > 0x0000000000000000000000000000000000000000 && b);
    }
}
// ----
// Warning: (362-421): Assertion violation happens here
