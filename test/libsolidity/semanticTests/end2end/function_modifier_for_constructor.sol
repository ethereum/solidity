
		contract A {
			uint data;
			constructor() mod1 public { data |= 2; }
			modifier mod1 virtual { data |= 1; _; }
			function getData() public returns (uint r) { return data; }
		}
		contract C is A {
			modifier mod1 override { data |= 4; _; }
		}
	
// ----
// getData() -> 0x6

