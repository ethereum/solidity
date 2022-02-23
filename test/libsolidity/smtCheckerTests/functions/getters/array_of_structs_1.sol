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
// Info 1180: Contract invariant(s) for :D:\n(items[1][2][3].y <= 0)\n
