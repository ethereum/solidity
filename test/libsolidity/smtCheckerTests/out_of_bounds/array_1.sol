contract C {
	uint[] a;
	uint l;
	function p() public {
		require(a.length < type(uint).max - 1);
		a.push();
		++l;
	}
	function q() public {
		require(a.length > 0);
		a.pop();
		--l;
	}
	function r() public view returns (uint) {
		require(l > 0);
		return a[l]; // oob access
	}
}
// ====
// SMTEngine: all
// SMTTargets: outOfBounds
// ----
// Warning 6368: (259-263): CHC: Out of bounds access happens here.\nCounterexample:\na = [0], l = 1\n = 0\n\nTransaction trace:\nC.constructor()\nState: a = [], l = 0\nC.p()\nState: a = [0], l = 1\nC.r()
