contract C {
	constructor() payable {
		assert(address(this).balance == msg.value); // should fail because there might be funds from before deployment
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (40-82): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor(){ msg.value: 0 }
