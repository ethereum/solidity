contract C
{
	function f() public view {
		address a = msg.sender;
		address b = msg.sender;
		assert(a == b);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
