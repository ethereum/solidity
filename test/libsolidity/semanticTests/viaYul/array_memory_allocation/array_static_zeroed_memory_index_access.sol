contract C {
	uint test1;
	uint test2;
	uint test3;
	uint test4;
	uint test5;
	uint test6;
	uint test7;
	mapping (string => uint) map;
	function set(string memory s) public returns (uint) {
		map[s] = 0;
		uint[3] memory x;
		return x[2];
	}
}
// ====
// compileViaYul: also
// ----
// set(string): 0x20, 32, "01234567890123456789012345678901" -> 0
