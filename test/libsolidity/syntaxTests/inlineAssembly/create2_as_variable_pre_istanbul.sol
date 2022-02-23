contract c {
	function f() public {
		uint create2; create2;
		assembly { pop(create2(0, 0, 0, 0)) }
	}
}
// ====
// EVMVersion: =byzantium
// ----
// TypeError 6166: (78-85): The "create2" instruction is only available for Constantinople-compatible VMs (you are currently compiling for "byzantium").
// TypeError 3950: (78-97): Expected expression to evaluate to one value, but got 0 values instead.
