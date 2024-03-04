contract C
{
	function uint_to_uint(uint8 u8, uint256 u256) public pure {
		uint256 b = uint256(u8);
		uint8 c = uint8(u8);
		b = uint256(u256);
		uint256 d = 1;
		c = uint8(d); // no truncation
		c = uint8(u256); // big cast to small - truncation
	}

	function fixed_bytes_to_fixed_bytes(bytes2 b2, bytes32 b32) public pure {
		bytes32 c = bytes32(b2);
		bytes2 d = bytes2(b2);
		c = bytes32(b32);

		d = bytes2(b32); // big cast to small - truncation
	}

	function array_to_fixed_bytes(bytes calldata c, bytes memory m) public pure returns (bytes16) {
		require(c.length == 16, "");
		bytes16 b = bytes16(c); // no truncation
		b = bytes16(m); // truncation possible
		return b;
	}

	function array_slice_to_fixed_bytes(bytes calldata c) public pure returns (bytes16) {
		require(c.length == 16, "");
		bytes8 b = bytes8(c[:7]); // not supported in BMC
		b = bytes8(c[:10]); // not supported in BMC
		return b;
	}

	function address_conversion_for_contract() public view returns (bytes20) {
		return bytes20(address(this)); // truncation not reported
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 3260: (201-212): BMC: Truncated value in type conversion happens here.
// Warning 3260: (406-417): BMC: Truncated value in type conversion happens here.
// Warning 3260: (634-644): BMC: Truncated value in type conversion happens here.
// Info 6002: BMC: 9 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
