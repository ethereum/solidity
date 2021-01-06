pragma experimental SMTChecker;
contract C {
	function g() public returns (uint) {
		try this.g() returns (uint x) { x; }
		catch Error(string memory s) { s; }
	}
}
// ====
// EVMVersion: >=byzantium
// ----
// Warning 6321: (75-79): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
