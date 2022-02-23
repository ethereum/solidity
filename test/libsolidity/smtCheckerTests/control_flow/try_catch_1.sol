contract C {
	function g() public returns (uint) {
		try this.g() returns (uint x) { x; }
		catch Error(string memory s) { s; }
	}
}
// ====
// EVMVersion: >=byzantium
// SMTEngine: all
// ----
// Warning 6321: (43-47): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
