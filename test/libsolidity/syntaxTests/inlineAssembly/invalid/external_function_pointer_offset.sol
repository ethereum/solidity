contract C {
	function testFunction() external {}

	function testYul() public {
		function() external fp = this.testFunction;

		uint myOffset;

		assembly {
			myOffset := fp.offset
		}
	}
}
// ----
// TypeError 9272: (173-182): Variables of type function pointer only support ".selector" and ".address".
