bytes constant a = "\x03\x01\x02";
bytes constant b = hex"030102";
string constant c = "hello";
uint256 constant x = 56;
enum ActionChoices {GoLeft, GoRight, GoStraight, Sit}
ActionChoices constant choices = ActionChoices.GoRight;
bytes32 constant st = "abc\x00\xff__";

contract C {
	function f() internal pure returns (bytes memory) {
		return a;
	}

	function g() internal pure returns (bytes memory) {
		return b;
	}

	function h() internal pure returns (bytes memory) {
		return bytes(c);
	}

	function i() internal pure returns (uint, ActionChoices, bytes32) {
		return (x, choices, st);
	}

	function p() public pure {
		assert(f().length == 3); // should hold
		assert(g().length == 3); // should hold
		assert(h().length == 5); // should hold
		(uint w, ActionChoices z, bytes32 t) = i();
		assert(x == 56); // should hold
		assert(w == 56); // should hold
		assert(z == ActionChoices.GoRight); // should hold
		assert(t == "abc\x00\xff__"); // should hold
		assert(w == 59); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (968-983): CHC: Assertion violation happens here.\nCounterexample:\n\nw = 56\nz = 1\nt = 0x61626300ff5f5f00000000000000000000000000000000000000000000000000\n\nTransaction trace:\nC.constructor()\nC.p()\n    C.f() -- internal call\n    C.g() -- internal call\n    C.h() -- internal call\n    C.i() -- internal call
