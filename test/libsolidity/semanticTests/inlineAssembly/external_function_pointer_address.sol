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
// testYul() -> 0x0fdd67305928fcac8d213d1e47bfa6165cd0b87b
// testSol() -> 0x0fdd67305928fcac8d213d1e47bfa6165cd0b87b
