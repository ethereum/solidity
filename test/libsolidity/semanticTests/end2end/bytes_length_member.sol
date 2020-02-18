
		contract c {
			function set() public returns (bool) { data = msg.data; return true; }
			function getLength() public returns (uint) { return data.length; }
			bytes data;
		}
	
// ----
// getLength() -> 0x0
// set(): 0x1, 0x2 -> 0x1
// getLength() -> 0x44

