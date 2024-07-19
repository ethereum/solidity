interface I {
	function f() external payable;
}

contract C {
	function g(I i) public {
		require(address(this).balance == 100);
		i.f{value: 0}();
		assert(address(this).balance == 100); // should hold
		assert(address(this).balance == 90); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTIgnoreCex: yes
// SMTSolvers: eld
// ----
// Warning 6328: (205-240): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
