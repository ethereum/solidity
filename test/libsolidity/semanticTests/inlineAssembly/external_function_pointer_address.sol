contract C {
	function testFunction() external {}

	function testYul() public returns (address adr) {
		function() external fp = this.testFunction;

		assembly {
			adr := fp.address
		}
	}
	function testSol() public returns (address) {
		return this.testFunction.address;
	}
}
// ----
// testYul() -> 0xc06afe3a8444fc0004668591e8306bfb9968e79e
// testSol() -> 0xc06afe3a8444fc0004668591e8306bfb9968e79e
