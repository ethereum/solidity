pragma experimental SMTChecker;

contract C {
	function f1() public pure {
		// signed <- signed
		int8 z = int8(-1);
		assert(z == -1);
		z = int8(0xf0ff);
		assert(z == -1);
		z = int8(0xcafecafef0ff);
		assert(z == -1);
		z = int8(0xcafecafe);
		assert(z == -2);
		z = int8(255);
		assert(z == -1);

		// unsigned <= unsigned
		uint8 x = uint8(uint16(-1));
		assert(x == 255);
		x = uint8(uint256(-1));
		assert(x == 255);

		// signed <- unsigned
		int8 y = int8(uint16(-1));
		assert(y == -1);
		y = int8(uint16(100));
		assert(y == 100);
		y = int8(uint16(200));
		assert(y == -56);

		// unsigned <- signed
		uint8 v = uint8(int16(-1));
		assert(v == 255);
		v = uint8(int16(300));
		assert(v == 44);
		v = uint8(int16(200));
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
// ----
