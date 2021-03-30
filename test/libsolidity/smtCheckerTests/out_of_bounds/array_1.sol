pragma experimental SMTChecker;

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
// ----
// Warning 4984: (145-148): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 3944: (214-217): CHC: Underflow (resulting value less than 0) might happen here.
// Warning 6368: (292-296): CHC: Out of bounds access happens here.
// Warning 2661: (145-148): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 4144: (214-217): BMC: Underflow (resulting value less than 0) happens here.
