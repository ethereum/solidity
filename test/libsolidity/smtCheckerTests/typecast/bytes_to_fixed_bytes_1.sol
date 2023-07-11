contract C {
	function f() external pure {
		bytes memory b = hex"00010203040506070809000102030405060708090001020304050607080900010203040506070809";
		bytes8 c = bytes8(b);
		assert(c == 0x0001020304050607); // should hold
		bytes16 d = bytes16(b);
		assert(d == 0x00010203040506070809000102030405);
		bytes24 e = bytes24(b);
		assert(e == 0x000102030405060708090001020304050607080900010203); // should hold
		bytes32 g = bytes32(b);
		assert(g == 0x0001020304050607080900010203040506070809000102030405060708090001); // should hold
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Info 1391: CHC: 4 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
