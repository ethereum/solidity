contract C {
	function callMeMaybe(uint a, uint b, uint[] memory c) external {}

	function abiEncodeSimple(uint x, uint y, uint z, uint[] memory a, uint[] memory b) public view {
		require(x == y);
		bytes memory b1 = abi.encodeCall(this.callMeMaybe, (x, z, a));
		bytes memory b2 = abi.encodeCall(this.callMeMaybe, (y, z, a));
		assert(b1.length == b2.length);

		bytes memory b3 = abi.encodeCall(this.callMeMaybe, (y, z, b));
		assert(b1.length == b3.length); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6031: (233-249): Internal error: Expression undefined for SMT solver.
// Warning 6031: (298-314): Internal error: Expression undefined for SMT solver.
// Warning 6031: (398-414): Internal error: Expression undefined for SMT solver.
// Warning 1218: (330-360): CHC: Error trying to invoke SMT solver.
// Warning 1218: (430-460): CHC: Error trying to invoke SMT solver.
// Warning 6328: (330-360): CHC: Assertion violation might happen here.
// Warning 6328: (430-460): CHC: Assertion violation might happen here.
// Warning 4661: (330-360): BMC: Assertion violation happens here.
// Warning 4661: (430-460): BMC: Assertion violation happens here.
