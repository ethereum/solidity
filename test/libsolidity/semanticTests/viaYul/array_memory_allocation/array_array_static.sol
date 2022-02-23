contract C {
	uint test1;
	uint test2;
	uint test3;
	uint test4;
	uint test5;
	uint test6;
	uint test7;
	mapping (string => uint) map;
	function set(string memory s, uint n, uint m) public returns (uint) {
		map[s] = 0;
		uint[4][] memory x = new uint[4][](n);
		return x[m][0];
	}
}
// ====
// compileViaYul: also
// ----
// set(string,uint256,uint256): 0x60, 2, 0, 32, "01234567890123456789012345678901" -> 0
// set(string,uint256,uint256): 0x60, 2, 1, 32, "01234567890123456789012345678901" -> 0
// set(string,uint256,uint256): 0x60, 2, 2, 32, "01234567890123456789012345678901" -> FAILURE, hex"4e487b71", 0x32
// set(string,uint256,uint256): 0x60, 200, 199, 32, "01234567890123456789012345678901" -> 0
