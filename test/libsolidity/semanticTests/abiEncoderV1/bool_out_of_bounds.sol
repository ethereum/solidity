pragma abicoder v1;
contract C {
	function f(bool b) public pure returns (bool) { return b; }
}
// ====
// ABIEncoderV1Only: true
// compileViaYul: false
// ----
// f(bool): true -> true
// f(bool): false -> false
// f(bool): 0x000000 -> false
// f(bool): 0xffffff -> true