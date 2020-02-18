
		contract Base {
			uint data;
			function setData(uint i) public { data = i; }
			function getViaBase() public returns (uint i) { return data; }
		}
		contract A is Base { function setViaA(uint i) public { setData(i); } }
		contract B is Base { function getViaB() public returns (uint i) { return getViaBase(); } }
		contract Derived is Base, B, A { }
	
// ====
// optimize-yul: false
// ----
// getViaB() -> 0x0
// setViaA(uint256): 0x17 -> 
// getViaB() -> 0x17

