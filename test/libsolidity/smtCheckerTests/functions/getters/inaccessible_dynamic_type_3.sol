contract C {
	struct S {
		string a;
	}
	S public s;
	function g() public view returns (uint256) {
		this.s();
	}
}
// ====
// SMTEngine: all
// EVMVersion: <=spuriousDragon
// ----
// Warning 6321: (88-95): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
