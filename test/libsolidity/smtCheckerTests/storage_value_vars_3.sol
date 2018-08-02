pragma experimental SMTChecker;
contract C
{
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
        assert(b == (a < 0x0000000000000000000000000000000000000200));
    }

    function g() public view {
        require(a < 0x0000000000000000000000000000000000000100);
        assert(c >= 0);
    }
    address a;
    bool b;
    uint c;
}
