
		contract test {
			function f(uint k) public returns(uint){
				return k;
		}
		}
	
// ====
// optimize-yul: false
// ----
// f(uint256): 0x9 -> 0x9

