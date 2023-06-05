contract C {
	function f() public returns (uint) {
		while(1==1)
			try this.f() returns (uint b) {
				b = 2;
			} catch {
			}
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6321: (43-47): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
