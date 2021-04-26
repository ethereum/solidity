pragma abicoder v2;
contract Test {
	struct MemoryUint {
		uint field;
	}
	function test() public pure returns (uint) {
		uint[] memory before = new uint[](1); // at offset 0x80
		// Two problems here: The first offset is zero, the second offset is missing.
		bytes memory corrupt = abi.encode(uint(32), // offset to "tuple"
										  uint(0)); // bogus first element
		/*
		  At this point the free pointer is 0x80 + 64 (size of before) + 32 (length field of corrupt) + 64 (two encoded words)

		  Now let's put random junk into memory immediately after the bogus first element. Our goal is to overflow the read pointer to point to before.
		  The value read out at this point will be added to beginning of the encoded tuple, AKA corrupt + 64. We need then to write x where:
		  x + 0x80 + 64 (before) + 32 (length of corrupt) + 32 (first word of corrupt) = 0x80 (mod 2^256)
		  that is MAX_UINT - 128
		*/
		MemoryUint memory afterCorrupt;
		afterCorrupt.field = uint(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff80);
		before[0] = 123456;
		uint[][2] memory decoded = abi.decode(corrupt, (uint[][2]));
		return decoded[1][0];
	}
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// test() -> FAILURE
