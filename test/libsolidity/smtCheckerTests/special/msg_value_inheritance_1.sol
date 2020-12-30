pragma experimental SMTChecker;

contract A {
	uint public x = msg.value;
	constructor() payable {
		assert(x == 0); // should fail, A can be constructed with any msg.value
	}
}

contract C is A {
	uint public v = msg.value;
	constructor() A() {
		assert(v == 0); // should hold
	}
}
// ----
// Warning 6328: (101-115): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n\nTransaction trace:\nA.constructor(){ value: 1 }
