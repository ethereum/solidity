contract C {
	constructor() payable {
		require(msg.value > 100);
	}
	function f() public view {
		assert(address(this).balance > 100); // should hold
		assert(address(this).balance > 200); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (153-188): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
