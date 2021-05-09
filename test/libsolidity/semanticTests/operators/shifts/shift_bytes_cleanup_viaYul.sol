contract C {
	function l(uint8 y) public returns (bytes20) {
		bytes20 x;
		assembly { x := "12345678901234567890abcde" }
		// When compiling via IR, `x` is truncated before applying
		// the operation.
		return x << y;
	}
	function r(uint8 y) public returns (bytes20) {
		bytes20 x;
		assembly { x := "12345678901234567890abcde" }
		return x >> y;
	}
}
// ====
// compileViaYul: true
// compileToEwasm: also
// ----
// l(uint8): 64 -> 0x3930313233343536373839306162636465000000000000000000000000000000
// r(uint8): 64 -> 0x313233343536373839303132000000000000000000000000
