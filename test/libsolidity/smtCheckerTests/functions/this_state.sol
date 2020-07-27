pragma experimental SMTChecker;

contract C
{
    uint public x;
    function g() public {
		x = 0;
        this.h();
		// Fails as false positive because CHC does not support `this`.
		assert(x == 2);
    }
    function h() public {
        x = 2;
    }
}
// ----
// Warning 6328: (186-200): Assertion violation happens here
