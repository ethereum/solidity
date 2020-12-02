pragma experimental SMTChecker;

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
// ----
// Warning 6328: (76-105): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (403-432): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (543-572): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nh()
