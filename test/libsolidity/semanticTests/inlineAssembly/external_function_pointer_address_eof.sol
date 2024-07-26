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
// ====
// compileToEOF: true
// EVMVersion: >=prague
// ----
// testYul() -> 0xb546c6ff998ecadc80f48650a3d77fd361aebb4e
// testSol() -> 0xb546c6ff998ecadc80f48650a3d77fd361aebb4e
