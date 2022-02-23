contract c {
	function f() public view {
		uint extcodehash;
		extcodehash;
		assembly { pop(extcodehash(0)) }
	}
}
// ====
// EVMVersion: =byzantium
// ----
// TypeError 7110: (93-104): The "extcodehash" instruction is only available for Constantinople-compatible VMs (you are currently compiling for "byzantium").
// TypeError 3950: (93-107): Expected expression to evaluate to one value, but got 0 values instead.
