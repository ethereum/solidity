contract C {
	function f(int x, int y) public pure returns (int) {
		require(x == 0);
		require(y == 0);
		return x + y;
	}
}
// ====
// SMTEngine: bmc
// SMTShowProvedSafe: yes
// ----
// Info 2961: (114-119): BMC: Underflow (resulting value less than -2**255) check is safe!
// Info 2961: (114-119): BMC: Overflow (resulting value larger than 2**255 - 1) check is safe!
