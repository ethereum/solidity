contract C {
	function testFunction() internal {}

	function testYul() public returns (uint32) {
		function() internal fp = testFunction;
		uint selectorValue = 0;

		assembly {
			selectorValue := fp.selector
		}

		return uint32(bytes4(bytes32(selectorValue)));
	}
	function testSol() public returns (uint32) {
		return uint32(testFunction.selector);
	}
}
// ----
// TypeError 8533: (198-209): Only Variables of type external function pointer support ".selector" and ".address".
// TypeError 9582: (329-350): Member "selector" not found or not visible after argument-dependent lookup in function ().
