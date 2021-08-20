contract C {
	constructor() payable {
		require(msg.value > 100);
	}
	uint c;
	function f(address payable _a, uint _v) public {
		require(_v < 10);
		require(c < 2);
		++c;
		_a.transfer(_v);
	}
	function inv() public view {
		assert(address(this).balance > 80); // should hold
		assert(address(this).balance > 90); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (280-314): CHC: Assertion violation happens here.\nCounterexample:\nc = 2\n\nTransaction trace:\nC.constructor(){ value: 101 }\nState: c = 0\nC.f(15, 6)\nState: c = 1\nC.f(5599, 8)\nState: c = 2\nC.inv()
// Warning 1236: (175-190): BMC: Insufficient funds happens here.
