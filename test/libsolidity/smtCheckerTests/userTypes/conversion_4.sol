pragma abicoder v2;

type MyUInt8 is uint8;
type MyInt8 is int8;
type MyUInt16 is uint16;

contract C {
	function m(MyUInt16 a) internal pure returns (MyUInt8) {
		return MyUInt8.wrap(uint8(MyUInt16.unwrap(a)));
	}

	function w() public pure {
		assert(MyUInt8.unwrap(m(MyUInt16.wrap(1))) == 1);
		assert(MyUInt8.unwrap(m(MyUInt16.wrap(2))) == 2);
		assert(MyUInt8.unwrap(m(MyUInt16.wrap(255))) == 0xff);
		assert(MyUInt8.unwrap(m(MyUInt16.wrap(255))) == 1); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (407-457): CHC: Assertion violation happens here.
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
