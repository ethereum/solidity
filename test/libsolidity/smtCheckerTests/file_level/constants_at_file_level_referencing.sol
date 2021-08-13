==== Source: s1.sol ====


bytes constant a = b;
bytes constant b = hex"030102";

function fre() pure returns (bytes memory) {
	return a;
}

==== Source: s2.sol ====

import "s1.sol";

uint256 constant c = uint8(a[0]) + 2;

contract C {
	function f() internal pure returns (bytes memory) {
		return a;
	}

	function g() internal pure returns (bytes memory) {
		return b;
	}

	function h() internal pure returns (uint) {
		return c;
	}

	function i() internal pure returns (bytes memory) {
		return fre();
	}

	function p() public pure {
		bytes memory r1 = f();
		assert(r1[0] == 0x03); // should hold
		assert(r1[1] == 0x01); // should hold
		assert(r1[2] == 0x02); // should hold
		assert(r1[2] == 0x04); // should fail

		bytes memory r2 = g();
		assert(r2[0] == 0x03); // should hold
		assert(r2[1] == 0x01); // should hold
		assert(r2[2] == 0x02); // should hold
		assert(r2[2] == 0x04); // should fail

		bytes memory r3 = i();
		assert(r3[0] == 0x03); // should hold
		assert(r3[1] == 0x01); // should hold
		assert(r3[2] == 0x02); // should hold
		assert(r3[2] == 0x04); // should fail

		uint z = h();
		assert(z == 5); // should hold
		assert(z == 7); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (s2.sol:518-539): CHC: Assertion violation happens here.
// Warning 6328: (s2.sol:704-725): CHC: Assertion violation happens here.
// Warning 6328: (s2.sol:890-911): CHC: Assertion violation happens here.
// Warning 6328: (s2.sol:980-994): CHC: Assertion violation happens here.
