
		contract test {
			constructor() public payable {}
			function getBalance() public returns (uint256 balance) {
				return address(this).balance;
			}
		}
	
// ====
// compileViaYul: also
// ----
// constructor(), 23 ether ->
// getBalance() -> 23000000000000000000
