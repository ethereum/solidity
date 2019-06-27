contract C {
	function test(uint256 len, uint idx) public returns (uint256)
	{
		uint[] memory array = new uint[](len);
		uint result = receiver(array, idx);

		for (uint256 i = 0; i < array.length; i++)
			require(array[i] == i + 1);

		return result;
	}
	function receiver(uint[] memory array, uint idx) public returns (uint256)
	{
		for (uint256 i = 0; i < array.length; i++)
			array[i] = i + 1;

		return array[idx];
	}
}
// ====
// compileViaYul: true
// ----
// test(uint256,uint256): 0,0 -> FAILURE
// test(uint256,uint256): 1,0 -> 1
// test(uint256,uint256): 10,5 -> 6
// test(uint256,uint256): 10,50 -> FAILURE
