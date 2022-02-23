contract C {
    struct S {
        mapping(address => uint) innerM;
		uint sum;
    }

	function f(S storage m, address i, uint v) internal {
		m.innerM[i] = v;
		m.sum += v;
	}

	S s;

	constructor(uint amt) {
		f(s, msg.sender, amt);
	}
	function g() public view {
		assert(s.sum == 0); // should hold but no aliasing support means it fails for now
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (270-288): CHC: Assertion violation happens here.\nCounterexample:\ns = {innerM, sum: 21239}\n\nTransaction trace:\nC.constructor(0){ msg.sender: 0x6dc4 }\nState: s = {innerM, sum: 21239}\nC.g()
