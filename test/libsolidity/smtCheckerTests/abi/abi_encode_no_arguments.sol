contract C {
	function f() pure public {
		bytes memory res = abi.encode();
		assert(res.length == 0); // should hold
		assert(res.length > 0); // should fail
		res = abi.encodePacked();
		assert(res.length == 0); // should hold
		assert(res.length > 0); // should fail
		res = abi.encodeWithSelector(0);
		assert(res.length == 4); // should hold, but SMTChecker cannot know this yet
		res = abi.encodeWithSignature("");
		assert(res.length == 4); // should hold, but SMTChecker cannot know this yet
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (120-142='assert(res.length > 0)'): CHC: Assertion violation happens here.
// Warning 6328: (231-253='assert(res.length > 0)'): CHC: Assertion violation happens here.
// Warning 6328: (307-330='assert(res.length == 4)'): CHC: Assertion violation happens here.
// Warning 6328: (423-446='assert(res.length == 4)'): CHC: Assertion violation happens here.
