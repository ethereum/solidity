pragma experimental SMTChecker;

interface I1 {
}

interface I2 {
	function f() external;
}

interface I3 {
	function f() external;
	function g(uint, address) external;
}

contract C {
	function f() public pure {
		assert(type(I1).interfaceId == 0);
		assert(type(I2).interfaceId != 0);
		assert(type(I2).interfaceId == 0x26121ff0);
		assert(type(I2).interfaceId != 0);
		assert(type(I3).interfaceId == 0x822b51c6);
	}
	function g() public pure {
		assert(type(I1).interfaceId == type(I2).interfaceId);
	}
	function h() public pure {
		assert(type(I2).interfaceId == type(I3).interfaceId);
	}
}
// ----
// Warning 6328: (449-501): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\ng()
// Warning 6328: (536-588): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nh()
