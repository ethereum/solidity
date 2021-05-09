pragma abicoder v1;
struct Item {
	uint x;
	uint y;
}

contract D {
	Item[] public items;

	function test() public {
		delete items;
		items.push(Item(42, 43));
		(uint a, uint b) = this.items(0);
		assert(a == 42); // should hold
		assert(b == 43); // should hold
		assert(b == 0); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (267-281): CHC: Assertion violation happens here.\nCounterexample:\nitems = [{x: 42, y: 43}]\na = 42\nb = 43\n\nTransaction trace:\nD.constructor()\nState: items = []\nD.test()
