
		contract Base {
			uint dataBase;
			function getViaBase() public returns (uint i) { return dataBase; }
		}
		contract Derived is Base {
			uint dataDerived;
			function setData(uint base, uint derived) public returns (bool r) {
				dataBase = base;
				dataDerived = derived;
				return true;
			}
			function getViaDerived() public returns (uint base, uint derived) {
				base = dataBase;
				derived = dataDerived;
			}
		}
	
// ====
// optimize-yul: false
// ----
// setData(uint256,uint256): 0x1, 0x2 -> 0x1
// getViaBase() -> 0x1
// getViaDerived() -> 0x1, 0x2

