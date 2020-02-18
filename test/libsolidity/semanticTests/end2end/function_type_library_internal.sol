
		library Utils {
			function reduce(uint[] memory array, function(uint, uint) internal returns (uint) f, uint init) internal returns (uint) {
				for (uint i = 0; i < array.length; i++) {
					init = f(array[i], init);
				}
				return init;
			}
			function sum(uint a, uint b) internal returns (uint) {
				return a + b;
			}
		}
		contract C {
			function f(uint[] memory x) public returns (uint) {
				return Utils.reduce(x, Utils.sum, 0);
			}
		}
	
// ----
// f(uint256[]): 0x20, 0x3, 0x01, 0x07, 0x03 -> 0x0b

