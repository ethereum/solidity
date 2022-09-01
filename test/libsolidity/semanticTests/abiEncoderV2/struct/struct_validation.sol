pragma abicoder               v2;

contract C {
	struct S { int16 a; uint8 b; bytes2 c; }
	function f(S memory s) public pure returns (uint a, uint b, uint c) {
		assembly {
			a := mload(s)
			b := mload(add(s, 0x20))
			c := mload(add(s, 0x40))
		}
	}
}
// ====
// compileToEwasm: also
// ----
// f((int16,uint8,bytes2)): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff01, 0xff, "ab" -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff01, 0xff, "ab"
// f((int16,uint8,bytes2)): 0xff010, 0xff, "ab" -> FAILURE
// f((int16,uint8,bytes2)): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff01, 0xff0002, "ab" -> FAILURE
// f((int16,uint8,bytes2)): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff01, 0xff, "abcd" -> FAILURE
