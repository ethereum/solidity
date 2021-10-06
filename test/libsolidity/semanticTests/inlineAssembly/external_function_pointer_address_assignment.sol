contract C {
	function testFunction() external {}

	function testYul(address newAddress) view public returns (address adr) {
		function() external fp = this.testFunction;

		assembly {
			fp.address := newAddress
		}

		return fp.address;
	}
}
// ====
// compileViaYul: also
// ----
// testYul(address): 0x1234567890 -> 0x1234567890
// testYul(address): 0xC0FFEE3EA7 -> 0xC0FFEE3EA7
