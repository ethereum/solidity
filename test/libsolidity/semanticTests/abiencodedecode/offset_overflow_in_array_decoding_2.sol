pragma abicoder v2;
contract Test {
	struct MemoryTuple {
		uint field1;
		uint field2;
	}
	function withinArray() public pure returns (uint) {
		uint[] memory before = new uint[](1);
		bytes memory corrupt = abi.encode(uint(32),
										  uint(2));
		MemoryTuple memory afterCorrupt;
		before[0] = 123456;
		/*
		  As above, but in this case we are adding to:
		  0x80 + 64 (before) + 32 (length of corrupt) + 32 (offset) + 32 (field pointer)
		  giving MAX_UINT - 96
		*/
		afterCorrupt.field1 = uint(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff60);
		afterCorrupt.field2 = uint(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff60);
		uint[][] memory decoded = abi.decode(corrupt, (uint[][]));
		/*
		  Will return 123456 * 2, AKA before has been copied twice
		 */
		return decoded[0][0] + decoded[1][0];
	}
}
// ====
// compileToEwasm: also
// ----
// withinArray() -> FAILURE
