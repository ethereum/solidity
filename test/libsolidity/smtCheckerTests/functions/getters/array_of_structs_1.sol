pragma abicoder v1;
struct Item {
	uint x;
	uint y;
}

contract D {
	Item[][][] public items;

	function test() public view returns (uint) {
		(uint a, uint b) = this.items(1, 2, 3);
		return a + b;
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
