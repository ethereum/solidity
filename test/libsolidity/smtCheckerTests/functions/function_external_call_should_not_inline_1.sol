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
// Warning 5084: (198-208): Type conversion is not yet fully supported and might yield false positives.
