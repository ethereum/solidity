pragma experimental SMTChecker;
contract C {
	function f() public returns (uint) {
		while(1==1)
			try this.f() returns (uint b) {
				b = 2;
			} catch {
			}
	}
}
// ----
// Warning 6321: (75-79): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6838: (91-95): BMC: Condition is always true.
