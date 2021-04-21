contract c {
	function f() public view {
		uint extcodehash;
		extcodehash;
		assembly { pop(extcodehash(0)) }
	}
}
// ====
// EVMVersion: >=constantinople
// ----
