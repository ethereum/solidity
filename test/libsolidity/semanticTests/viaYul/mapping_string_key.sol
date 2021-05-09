contract C {
	mapping (string => uint) map;
	function set(string memory s) public {
		map[s];
	}
}
// ====
// compileViaYul: also
// ----
// set(string): 0x20, 32, "01234567890123456789012345678901" ->
