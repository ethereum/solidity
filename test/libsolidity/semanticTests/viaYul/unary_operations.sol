contract C {
	function preincr_u8(uint8 a) public pure returns (uint8) {
		return ++a + a;
	}
	function postincr_u8(uint8 a) public pure returns (uint8) {
		return a++ + a;
	}
	function predecr_u8(uint8 a) public pure returns (uint8) {
		return --a + a;
	}
	function postdecr_u8(uint8 a) public pure returns (uint8) {
		return a-- + a;
	}
	function preincr_s8(int8 a) public pure returns (int8 ret1, int8 ret2) {
		ret1 = ++a;
		ret2 = a;
	}
	function postincr_s8(int8 a) public pure returns (int8 ret1, int8 ret2) {
		ret1 = a++;
		ret2 = a;
	}
	function predecr_s8(int8 a) public pure returns (int8 ret1, int8 ret2) {
		ret1 = --a;
		ret2 = a;
	}
	function postdecr_s8(int8 a) public pure returns (int8 ret1, int8 ret2) {
		ret1 = a--;
		ret2 = a;
	}
	function preincr(uint a) public pure returns (uint) {
		return ++a + a;
	}
	function postincr(uint a) public pure returns (uint) {
		return a++ + a;
	}
	function predecr(uint a) public pure returns (uint) {
		return --a + a;
	}
	function postdecr(uint a) public pure returns (uint) {
		return a-- + a;
	}
	function not(bool a) public pure returns (bool)
	{
		return !a;
	}
	function bitnot(int256 a) public pure returns (int256)
	{
		return ~a;
	}
	function bitnot_u8(uint8 a) public pure returns (uint256 ret)
	{
		a = ~a;
		assembly {
			// Tests that the lower bit parts are cleaned up
			ret := a
		}
	}
	function bitnot_s8() public pure returns (int256 ret)
	{
		int8 a;
		assembly {
			a := 0x9C
		}

		a = ~a;

		assembly {
			// Tests that the lower bit parts are cleaned up
			ret := a
		}
	}
}
// ====
// compileViaYul: true
// ----
// preincr_s8(int8): 128 -> FAILURE
// postincr_s8(int8): 128 -> FAILURE
// preincr_s8(int8): 127 -> FAILURE
// postincr_s8(int8): 127 -> FAILURE
// preincr_s8(int8): 126 -> 127, 127
// postincr_s8(int8): 126 -> 126, 127
// predecr_s8(int8): -128 -> FAILURE
// postdecr_s8(int8): -128 -> FAILURE
// predecr_s8(int8): -127 -> -128, -128
// postdecr_s8(int8): -127 -> -127, -128
// preincr_s8(int8): -5 -> -4, -4
// postincr_s8(int8): -5 -> -5, -4
// predecr_s8(int8): -5 -> -6, -6
// postdecr_s8(int8): -5 -> -5, -6
// preincr_u8(uint8): 255 -> FAILURE
// postincr_u8(uint8): 255 -> FAILURE
// preincr_u8(uint8): 254 -> FAILURE
// postincr_u8(uint8): 254 -> FAILURE
// predecr_u8(uint8): 0 -> FAILURE
// postdecr_u8(uint8): 0 -> FAILURE
// predecr_u8(uint8): 1 -> 0
// postdecr_u8(uint8): 1 -> 1
// preincr_u8(uint8): 2 -> 6
// postincr_u8(uint8): 2 -> 5
// predecr_u8(uint8): 2 -> 2
// postdecr_u8(uint8): 2 -> 3
// preincr(uint256): 2 -> 6
// postincr(uint256): 2 -> 5
// predecr(uint256): 2 -> 2
// postdecr(uint256): 2 -> 3
// not(bool): true -> false
// not(bool): false -> true
// bitnot(int256): 5 -> -6
// bitnot(int256): 10 -> -11
// bitnot(int256): 0 -> -1
// bitnot(int256): -100 -> 99
// bitnot_u8(uint8): 100 -> 155
// bitnot_s8() -> 99
