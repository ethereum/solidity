contract C {
	struct S {
		uint8 x;
	}

	S s;

	constructor() {
		s.x = 254;
	}

	function inc_pre() public {
		++s.x;
	}

	function check() view public {
		uint y = s.x;
		assert(y < 256);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 4984: (112-117): CHC: Overflow (resulting value larger than 255) happens here.\nCounterexample:\ns = {x: 255}\n\nTransaction trace:\nC.constructor()\nState: s = {x: 254}\nC.inc_pre()\nState: s = {x: 255}\nC.inc_pre()
