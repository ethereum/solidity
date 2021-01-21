pragma experimental SMTChecker;
contract A {
	int[] a;
	constructor() {
		p();
	}
	function p() public {
		a.push(1);
	}
	function f() public {
		require(a.length == 1 && a[0] == 1);
		int[] storage u = a;
		assert(u[0] == 1); // should hold
		int[] memory b = new int[](2);
		a = b;
		// Access is safe but oob is reported due to aliasing.
		assert(u[0] == 1); // should fail
		A.a = b;
		// Access is safe but oob is reported due to aliasing.
		assert(u[0] == 1); // should fail
	}

	function push_v(int x) public {
		a.push(x);
	}
}
// ----
// Warning 6368: (350-354): CHC: Out of bounds access happens here.\nCounterexample:\na = [0, 0]\nu = []\nb = [0, 0]\n\nTransaction trace:\nA.constructor()\nState: a = [1]\nA.f()
// Warning 6328: (343-360): CHC: Assertion violation happens here.\nCounterexample:\na = [0, 0]\nb = [0, 0]\n\nTransaction trace:\nA.constructor()\nState: a = [1]\nA.f()
// Warning 6368: (454-458): CHC: Out of bounds access happens here.\nCounterexample:\na = [0, 0]\nu = []\nb = [0, 0]\n\nTransaction trace:\nA.constructor()\nState: a = [1]\nA.f()
// Warning 6328: (447-464): CHC: Assertion violation happens here.\nCounterexample:\na = [0, 0]\nb = [0, 0]\n\nTransaction trace:\nA.constructor()\nState: a = [1]\nA.f()
