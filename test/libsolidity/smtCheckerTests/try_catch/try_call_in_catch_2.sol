pragma experimental SMTChecker;
contract C {
    function f() public returns (uint, uint) {
        try this.f() {
        } catch Error(string memory) {
			g();
		}
	}
	function g() public pure {
		int test = 1;
	}
}
// ----
// Warning 6321: (78-82): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (84-88): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 2072: (199-207): Unused local variable.
