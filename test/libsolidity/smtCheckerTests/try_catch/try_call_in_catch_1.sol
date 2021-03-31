pragma experimental SMTChecker;
contract C {
    function f() public returns (uint) {
        try this.f() {
        } catch Error(string memory) {
			g();
		}
	}
	function g() public pure returns (address) {
	}
}
// ----
// Warning 6321: (78-82): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
