
		contract C {
			uint public result_in_constructor;
			function (uint) internal returns (uint) x;
			constructor() public {
				x = double;
				result_in_constructor = use(2);
			}
			function double(uint _arg) public returns (uint _ret) {
				_ret = _arg * 2;
			}
			function use(uint _arg) public returns (uint) {
				return x(_arg);
			}
		}
	
// ----
// use(uint256): 3 -> 6
// result_in_constructor() -> 4

