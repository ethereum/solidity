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
// Warning 7645: (98-121): Assertion checker does not support try/catch clauses.
// Warning 7645: (124-159): Assertion checker does not support try/catch clauses.
