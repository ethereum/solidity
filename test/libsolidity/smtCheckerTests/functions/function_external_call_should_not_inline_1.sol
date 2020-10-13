pragma experimental SMTChecker;
contract State {
	C c;
	function f() public returns (uint) {
		while(true)
			c.setOwner();
	}
}
contract C {
	address owner;
	function setOwner() public {
		owner = address(0);
	}
}
// ----
// Warning 6321: (85-89): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
