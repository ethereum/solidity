contract c {
	function f() public {
		uint create2; create2;
		assembly { pop(create2(0, 0, 0, 0)) }
	}
}
// ====
// EVMVersion: >=constantinople
// ----
