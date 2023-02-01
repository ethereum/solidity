contract C
{
	function f(uint prevrandao) public view {
		assert(block.prevrandao == prevrandao); // should fail
		assert(block.difficulty == prevrandao); // should fail
		assert(block.difficulty == block.prevrandao); // should hold
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 8417: (122-138): Since the VM version paris, "difficulty" was replaced by "prevrandao", which now returns a random number based on the beacon chain.
// Warning 8417: (179-195): Since the VM version paris, "difficulty" was replaced by "prevrandao", which now returns a random number based on the beacon chain.
// Warning 6328: (58-96): CHC: Assertion violation happens here.
// Warning 6328: (115-153): CHC: Assertion violation happens here.
