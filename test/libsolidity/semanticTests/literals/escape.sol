pragma experimental ABIEncoderV2;

contract C
{
	function f() public pure returns (uint, byte, byte) {
		bytes memory encoded = abi.encodePacked("\\\\");
		return (encoded.length, encoded[0], encoded[1]);
	}
}
// ====
// compileViaYul: also
// ----
// f() -> 2, 0x5c00000000000000000000000000000000000000000000000000000000000000, 0x5c00000000000000000000000000000000000000000000000000000000000000
