contract C {
	function create(uint256 len) public returns (uint256)
	{
		uint[] memory array = new uint[](len);
		return array.length;
	}
}
// ====
// compileViaYul: true
// ----
// create(uint256): 0 -> 0
// create(uint256): 7 -> 7
// create(uint256): 10 -> 10
