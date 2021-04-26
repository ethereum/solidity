contract C {
	uint[][] a;
	function p() public { a.push(); }
	function q(uint i) public {
		require(i < a.length);
		a[i].push();
	}
	function r(uint i, uint j) public view returns (uint) {
		require(i < a.length);
		return a[i][j]; // unsafe access
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6368: (224-231): CHC: Out of bounds access happens here.\nCounterexample:\na = [[]]\ni = 0\nj = 0\n = 0\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.p()\nState: a = [[]]\nC.r(0, 0)
