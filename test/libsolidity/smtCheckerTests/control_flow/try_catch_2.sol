contract C {
	function f() public {
		try this.f() {}
		catch (bytes memory x) {
			x;
		}
	}
}
// ====
// SMTEngine: all
// EVMVersion: >=byzantium
// ----
