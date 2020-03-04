contract C {
    function f() external view {}
	function test(address a) external view returns (bool status) {
		// This used to incorrectly raise an error about violating the view mutability.
		(status,) = a.staticcall.gas(42)("");
		(status,) = a.staticcall{gas: 42}("");
		this.f.gas(42)();
		this.f{gas: 42}();
	}
}
// ====
// EVMVersion: >=byzantium
// ----
// Warning: (207-223): Using ".gas(...)" is deprecated. Use "{gas: ...}" instead.
// Warning: (276-286): Using ".gas(...)" is deprecated. Use "{gas: ...}" instead.
