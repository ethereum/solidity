contract C {
    function f() external view {}
	function test(address a) external view returns (bool status) {
		// This used to incorrectly raise an error about violating the view mutability.
		(status,) = a.staticcall.gas(42)("");
		this.f.gas(42)();
	}
}
// ====
// EVMVersion: >=byzantium
// ----
