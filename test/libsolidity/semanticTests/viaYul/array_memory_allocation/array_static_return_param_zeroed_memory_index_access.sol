contract C {
	uint test1;
	uint test2;
	uint test3;
	uint test4;
	uint test5;
	uint test6;
	uint test7;
	mapping (string => uint) map;
	function set(string memory s) public returns (uint[3] memory x, uint[2] memory y, uint[] memory z, uint t) {
		map[s] = 0;
	}
}
// ====
// compileViaYul: also
// ----
// set(string): 0x20, 32, "01234567890123456789012345678901" -> 0, 0, 0, 0, 0, 0xe0, 0, 0
