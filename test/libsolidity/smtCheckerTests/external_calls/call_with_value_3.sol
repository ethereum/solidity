contract C {
	function g(address payable i) public {
		require(address(this).balance > 100);
		i.call{value: 20}("");
		assert(address(this).balance > 0); // should hold
		assert(address(this).balance == 0); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTIgnoreCex: yes
// SMTSolvers: eld
// ----
// Warning 9302: (95-116): Return value of low-level calls not used.
// Warning 6328: (172-206): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
