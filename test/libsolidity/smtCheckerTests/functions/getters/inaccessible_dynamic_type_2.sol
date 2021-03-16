pragma experimental SMTChecker;
contract C {
	function f() public returns(bool[]memory) {
		this.f();
	}
}
// ====
// EVMVersion: <=spuriousDragon
// ----
// Warning 6321: (74-86): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
