contract A {
	uint public x = msg.value;
	constructor() {
		assert(x == 0); // can fail when A is constructed as part of C
	}
}

contract B {
	constructor() payable {
		assert(msg.value >= 0); // should hold
	}
}
contract C is A, B {
	constructor() A() B() payable {
		assert(msg.value >= 0); // should hold
	}
}

// ====
// SMTEngine: all
// ----
// Warning 6328: (60-74): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
