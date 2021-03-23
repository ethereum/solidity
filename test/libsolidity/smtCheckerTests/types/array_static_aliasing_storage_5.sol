pragma experimental SMTChecker;

contract C
{
	uint[2] b1;
	uint[2] b2;
	function f(uint[2] storage a, uint[2] memory c) internal {
		// Accesses are safe but oob is reported because of aliasing.
		c[0] = 42;
		a[0] = 2;
		b1[0] = 1;
		// Erasing knowledge about storage variables should not
		// erase knowledge about memory references.
		assert(c[0] == 42);
		// Fails because b1 == a is possible.
		// Disabled because Spacer seg faults.
		//assert(a[0] == 2);
		assert(b1[0] == 1);
	}
	function g(bool x, uint[2] memory c) public {
		if (x) f(b1, c);
		else f(b2, c);
	}
}
// ====
// SMTIgnoreCex: yes
// ----
// Warning 6368: (198-202): CHC: Out of bounds access happens here.
// Warning 6368: (211-215): CHC: Out of bounds access happens here.
// Warning 6368: (223-228): CHC: Out of bounds access happens here.
// Warning 6368: (347-351): CHC: Out of bounds access happens here.
// Warning 6368: (473-478): CHC: Out of bounds access happens here.
