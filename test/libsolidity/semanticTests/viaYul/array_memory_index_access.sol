contract C {
	function index(uint256 len) public returns (bool)
	{
		uint[] memory array = new uint[](len);

		for (uint256 i = 0; i < len; i++)
			array[i] = i + 1;

		for (uint256 i = 0; i < len; i++)
			require(array[i] == i + 1, "Unexpected value in array!");

		return array.length == len;
	}
	function accessIndex(uint256 len, int256 idx) public returns (uint256)
	{
		uint[] memory array = new uint[](len);

		for (uint256 i = 0; i < len; i++)
			array[i] = i + 1;

		return array[uint256(idx)];
	}
}
// ----
// index(uint256): 0 -> true
// index(uint256): 10 -> true
// index(uint256): 20 -> true
// index(uint256): 0xFF -> true
// gas irOptimized: 108299
// gas legacy: 183064
// gas legacyOptimized: 117448
// accessIndex(uint256,int256): 10, 1 -> 2
// accessIndex(uint256,int256): 10, 0 -> 1
// accessIndex(uint256,int256): 10, 11 -> FAILURE, hex"4e487b71", 0x32
// accessIndex(uint256,int256): 10, 10 -> FAILURE, hex"4e487b71", 0x32
// accessIndex(uint256,int256): 10, -1 -> FAILURE, hex"4e487b71", 0x32
