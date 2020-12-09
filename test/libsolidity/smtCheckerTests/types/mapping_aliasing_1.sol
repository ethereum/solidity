pragma experimental SMTChecker;

contract C
{
	mapping (uint => uint) a;
	mapping (uint => uint) b;

	function f(uint x) public {
		a[1] = x;
		b[1] = x;
		a[1] = 2;
		mapping (uint => uint) storage c = a;
		assert(c[1] == 2);
		// False negative! Needs aliasing.
		assert(a[1] == b[1]);
	}
}
// ----
// Warning 6328: (266-286): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\n\n\nTransaction trace:\nconstructor()\nf(0)
