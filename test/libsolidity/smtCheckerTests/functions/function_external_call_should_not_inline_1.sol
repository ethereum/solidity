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
// ====
// SMTEngine: all
// ----
// Warning 6321: (53-57): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Info 6002: BMC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
