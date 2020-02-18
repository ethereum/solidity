
		contract C {
			function (uint) internal returns (uint) x;
			constructor() public {
				x = double;
			}
			function test() public returns (bool) {
				return x == double;
			}
			function double(uint _arg) public returns (uint _ret) {
				_ret = _arg * 2;
			}
		}
	
// ----
// test() -> 0x1

