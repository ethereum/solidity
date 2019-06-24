contract C {
	uint[] storageArray;
	function test_boundery_check(uint256 len, uint256 access) public returns
(uint256)
	{
		storageArray.length = len;
		return storageArray[access];
	}
}
// ====
// compileViaYul: true
// ----
// test_boundery_check(uint256, uint256): 10, 11 -> FAILURE
// test_boundery_check(uint256, uint256): 10, 9 -> 0
// test_boundery_check(uint256, uint256): 1, 9 -> FAILURE
// test_boundery_check(uint256, uint256): 1, 1 -> FAILURE
// test_boundery_check(uint256, uint256): 10, 10 -> FAILURE
// test_boundery_check(uint256, uint256): 256, 256 -> FAILURE
// test_boundery_check(uint256, uint256): 256, 255 -> 0
// test_boundery_check(uint256, uint256): 256, 0xFFFF -> FAILURE
// test_boundery_check(uint256, uint256): 256, 2 -> 0
