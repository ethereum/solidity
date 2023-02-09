pragma abicoder v2;

type MyUInt8 is uint8;
type MyInt8 is int8;
type MyUInt16 is uint16;

contract C {
	function j(MyUInt8 a) internal pure returns (uint) {
		return MyUInt8.unwrap(a);
	}
	function k(MyUInt8 a) internal pure returns (MyUInt16) {
		return MyUInt16.wrap(MyUInt8.unwrap(a));
	}

	function t() public pure {
		assert(j(MyUInt8.wrap(1)) == 1);
		assert(j(MyUInt8.wrap(2)) == 2);
		assert(j(MyUInt8.wrap(255)) == 0xff);
		assert(j(MyUInt8.wrap(255)) == 1); // should fail
	}

	function v() public pure {
		assert(MyUInt16.unwrap(k(MyUInt8.wrap(1))) == 1);
		assert(MyUInt16.unwrap(k(MyUInt8.wrap(2))) == 2);
		assert(MyUInt16.unwrap(k(MyUInt8.wrap(255))) == 0xff);
		assert(MyUInt16.unwrap(k(MyUInt8.wrap(255))) == 1); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (434-467): CHC: Assertion violation happens here.
// Warning 6328: (679-729): CHC: Assertion violation happens here.
// Info 1391: CHC: 6 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
