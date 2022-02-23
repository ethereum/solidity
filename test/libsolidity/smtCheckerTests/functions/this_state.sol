contract C
{
    uint public x;
    function g() public {
		x = 0;
        this.h();
		assert(x == 2);
    }
    function h() public {
        x = 2;
    }
}
// ====
// SMTEngine: all
// ----
// Info 1180: Contract invariant(s) for :C:\n((x = 0) || (x = 2))\n
