contract C {
    struct S {
        mapping(address => uint) innerM;
		uint sum;
    }

	function f(mapping(address => uint) storage innerM, address i, uint v) internal {
		innerM[i] = v;
	}

	S s;

	constructor(uint amt) {
		f(s.innerM, msg.sender, amt);
	}
	function g() public view {
		assert(s.innerM[msg.sender] == 0); // should hold but no aliasing support means it fails for now
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (289-322): CHC: Assertion violation happens here.\nCounterexample:\ns = {innerM, sum: 11}\n\nTransaction trace:\nC.constructor(0){ msg.sender: 0x6dc4 }\nState: s = {innerM, sum: 11}\nC.g(){ msg.sender: 0x0985 }
