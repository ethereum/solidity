contract C {
	uint test1;
	uint test2;
	uint test3;
	uint test4;
	uint test5;
	uint test6;
	uint test7;
	mapping (string => uint) map;
	function set(string memory s, uint n, uint a) public returns (uint) {
		map[s] = 0;
		uint[] memory x = new uint[](n);
		return x[a];
	}
}
// ====
// compileViaYul: also
// ----
// set(string,uint256,uint256): 0x60, 5, 0, 32, "01234567890123456789012345678901" -> 0
// set(string,uint256,uint256): 0x60, 5, 1, 32, "01234567890123456789012345678901" -> 0
// set(string,uint256,uint256): 0x60, 5, 4, 32, "01234567890123456789012345678901" -> 0
// set(string,uint256,uint256): 0x60, 5, 5, 32, "01234567890123456789012345678901" -> FAILURE
