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
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
