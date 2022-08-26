contract C {
	function f() external pure {
		bytes memory b = hex"00010203040506070809000102030405060708090001020304050607080900010203040506070809";
		bytes8 c = bytes8(b);
		//assert(c == 0x0001020304050607); // should hold
		assert(c == 0x0001020304050608); // should fail
		bytes16 d = bytes16(b);
		//assert(d == 0x00010203040506070809000102030405);
		assert(d == 0x00010203040506070809000102030406); // should fail
		bytes24 e = bytes24(b);
		//assert(e == 0x000102030405060708090001020304050607080900010203); // should hold
		assert(e == 0x000102030405060708090001020304050607080900010204); // should fail
		bytes32 g = bytes32(b);
		//assert(g == 0x0001020304050607080900010203040506070809000102030405060708090001); // should hold
		assert(g == 0x0001020304050607080900010203040506070809000102030405060708090002); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (227-258): CHC: Assertion violation happens here.
// Warning 6328: (356-403): CHC: Assertion violation happens here.
// Warning 6328: (532-595): CHC: Assertion violation happens here.
// Warning 6328: (740-819): CHC: Assertion violation happens here.
