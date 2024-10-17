contract C {
	function g(address payable i) public {
		require(address(this).balance == 100);
		i.call{value: 0}("");
		assert(address(this).balance == 100); // should hold
		assert(address(this).balance == 20); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTSolvers: eld
// ----
// Warning 9302: (96-116): Return value of low-level calls not used.
// Warning 6328: (175-210): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
