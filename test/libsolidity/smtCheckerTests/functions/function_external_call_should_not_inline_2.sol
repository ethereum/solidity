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
// ====
// SMTEngine: all
// ----
// Info 6002: BMC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
