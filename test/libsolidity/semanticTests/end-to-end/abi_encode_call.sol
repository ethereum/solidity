
		contract C {
			bool x;
			function c(uint a, uint[] memory b) public {
				require(a == 5);
				require(b.length == 2);
				require(b[0] == 6);
				require(b[1] == 7);
				x = true;
			}
			function f() public returns (bool) {
				uint a = 5;
				uint[] memory b = new uint[](2);
				b[0] = 6;
				b[1] = 7;
				(bool success,) = address(this).call(abi.encodeWithSignature("c(uint256,uint256[])", a, b));
				require(success);
				return x;
			}
		}
	
// ----
// f() -> true

