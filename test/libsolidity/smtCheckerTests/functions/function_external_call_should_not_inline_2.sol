pragma experimental SMTChecker;
contract Other {
	C c;
	function h(bool b) public {
		if (b)
			c.setOwner(address(0));
	}
}
contract C {
	address owner;
	function setOwner(address _owner) public {
		owner = _owner;
	}
}
// ----
// Warning 5084: (107-117): Type conversion is not yet fully supported and might yield false positives.
