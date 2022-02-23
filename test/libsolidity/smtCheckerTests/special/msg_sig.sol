contract C
{
	function f() public pure {
		assert(msg.sig == 0x00000000);
		assert(msg.sig == 0x26121ff0);
		fi();
		gi();
	}
	function fi() internal pure {
		assert(msg.sig == 0x26121ff0);
	}
	function g() public pure {
		assert(msg.sig == 0xe2179b8e);
		gi();
	}
	function gi() internal pure {
		// Fails since f can also call gi in which case msg.sig == 0x26121ff0
		assert(msg.sig == 0xe2179b8e);
	}
	function h() public pure {
		// Fails since gi can also call h in which case msg.sig can be f() or g()
		assert(msg.sig == 0xe2179b8e);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (43-72): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f(){ msg.sig: 0x26121ff0 }
// Warning 6328: (370-399): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f(){ msg.sig: 0x26121ff0 }\n    C.fi() -- internal call\n    C.gi() -- internal call
// Warning 6328: (510-539): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.h(){ msg.sig: 0xb8c9d365 }
