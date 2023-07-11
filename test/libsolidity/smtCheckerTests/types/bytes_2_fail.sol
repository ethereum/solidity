contract C
{
	function f(bytes memory b1, bytes memory b2) public pure {
		b1 = b2;
		require(b1.length > 2 && b2.length > 2);
		assert(b1[1] == b2[2]);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (129-151): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
