pragma experimental SMTChecker;

contract C {
	uint[][] a;
	function p() public { a.push(); }
	function q(uint i) public {
		require(i < a.length);
		a[i].push();
	}
	function r() public view {
		for (uint i = 0; i < a.length + 10; ++i)
			for (uint j = 0; j < a[i].length + 20; ++j)
				a[i][j]; // oob access
	}
}
// ----
// Warning 4984: (217-230): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 6368: (261-265): CHC: Out of bounds access happens here.\nCounterexample:\na = []\ni = 0\nj = 0\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.r()
// Warning 4984: (261-277): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 6368: (288-292): CHC: Out of bounds access happens here.\nCounterexample:\na = []\ni = 0\nj = 0\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.r()
// Warning 6368: (288-295): CHC: Out of bounds access happens here.\nCounterexample:\na = []\ni = 0\nj = 0\n\nTransaction trace:\nC.constructor()\nState: a = []\nC.r()
// Warning 4984: (279-282): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 4984: (232-235): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 2661: (217-230): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 2661: (261-277): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
