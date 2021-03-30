pragma experimental SMTChecker;
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
// SMTIgnoreCex: yes
// ----
// Warning 6328: (152-174): CHC: Assertion violation happens here.
// Warning 6328: (263-285): CHC: Assertion violation happens here.
// Warning 6328: (339-362): CHC: Assertion violation happens here.
// Warning 6328: (455-478): CHC: Assertion violation happens here.
