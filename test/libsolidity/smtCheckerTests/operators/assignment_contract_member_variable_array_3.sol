pragma experimental SMTChecker;
contract A {
	int[] a;
	function f() public {
		require(a.length == 1 && a[0] == 1);
		int[] storage u = a;
		assert(u[0] == 1); // should hold
		int[] memory b = new int[](2);
		a = b;
		assert(u[0] == 1); // should fail
		A.a = b;
		assert(u[0] == 1); // should fail
	}

	function push_v(int x) public {
		a.push(x);
	}
}
// ----
// Warning 6328: (220-237): CHC: Assertion violation happens here.
// Warning 6328: (267-284): CHC: Assertion violation happens here.
