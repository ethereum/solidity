pragma experimental SMTChecker;
contract C {
	function f() public {
		try this.f() {}
		catch (bytes memory x) {
			x;
		}
	}
}
// ====
// EVMVersion: >=byzantium
// ----
