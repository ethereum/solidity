pragma abicoder v1;
struct Item {
	uint x;
	uint y;
	uint[] arr;
}

contract D {
	Item[] public items;

	function test() public {
		delete items;
		uint[] memory tmp = new uint[](1);
		items.push(Item(42, 43, tmp));
		(uint a, uint b) = this.items(0);
		assert(a == 42); // should hold
		assert(b == 43); // should hold
		assert(b == 0); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (322-336): CHC: Assertion violation happens here.\nCounterexample:\nitems = [{x: 42, y: 43, arr: [0]}]\ntmp = [0]\na = 42\nb = 43\n\nTransaction trace:\nD.constructor()\nState: items = []\nD.test()
