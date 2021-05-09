contract C {
	function f() public returns(bool[]memory) {
		this.f();
	}
}
// ====
// SMTEngine: all
// EVMVersion: <=spuriousDragon
// ----
// Warning 6321: (42-54): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
