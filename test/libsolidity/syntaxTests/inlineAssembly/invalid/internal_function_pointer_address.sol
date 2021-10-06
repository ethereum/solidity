contract C {
	function testFunction() internal {}

	function testYul() public returns (address adr) {
		function() internal fp = testFunction;
		uint selectorValue = 0;

		assembly {
			adr := fp.address
		}
	}
	function testSol() public returns (address) {
		return testFunction.address;
	}
}
// ----
// TypeError 8533: (193-203): Only Variables of type external function pointer support ".selector" and ".address".
// TypeError 9582: (267-287): Member "address" not found or not visible after argument-dependent lookup in function ().
