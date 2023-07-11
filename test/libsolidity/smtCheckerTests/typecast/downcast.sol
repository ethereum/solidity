contract C {
	function f1() public pure {
		// signed <- signed
		int8 z = int8(-1);
		assert(z == -1);
		z = int8(int(0) - 1);
		assert(z == -1);
		z = int8(int(0) - 1);
		assert(z == -1);
		z = int8(int(0) - 2);
		assert(z == -2);
		z = int8(int(0) - 1);
		assert(z == -1);

		// unsigned <= unsigned
		uint8 x = uint8(type(uint256).max);
		assert(x == 255);
		x = uint8(type(uint256).max);
		assert(x == 255);

		// signed <- unsigned
		int8 y = int8(uint8(type(uint16).max));
		assert(y == -1);
		y = int8(uint8(uint16(100)));
		assert(y == 100);
		y = int8(uint8(uint16(200)));
		assert(y == -56);

		// unsigned <- signed
		uint8 v = uint8(type(uint16).max);
		assert(v == 255);
		v = uint8(int8(int16(300)));
		assert(v == 44);
		v = uint8(int8(int16(200)));
		assert(v == 200);

		// fixed bytes
		bytes2 b = bytes2(bytes4(0xcafeffff));
		assert(b == 0xcafe);
		b = bytes2(bytes4(bytes8(0xaaaabbbbccccdddd)));
		assert(b == 0xaaaa);
		b = bytes2(bytes8(0xaaaabbbbccccdddd));
		assert(b == 0xaaaa);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 20 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
