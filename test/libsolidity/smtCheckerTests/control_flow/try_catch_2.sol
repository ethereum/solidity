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
// Warning 7645: (83-85): Assertion checker does not support try/catch clauses.
// Warning 7645: (88-122): Assertion checker does not support try/catch clauses.
// Warning 7645: (83-85): Assertion checker does not support try/catch clauses.
// Warning 7645: (88-122): Assertion checker does not support try/catch clauses.
