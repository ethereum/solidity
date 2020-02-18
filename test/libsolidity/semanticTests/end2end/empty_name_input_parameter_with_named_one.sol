
		contract test {
			function f(uint, uint k) public returns(uint ret_k, uint ret_g){
				uint g = 8;
				ret_k = k;
				ret_g = g;
			}
		}
	
// ====
// optimize-yul: false
// ----
// f(uint256,uint256): 0x5, 0x9 -> 0x9, 0x8

