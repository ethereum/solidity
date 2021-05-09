contract C {
	function f() public {}
}
// ====
// EVMVersion: >=byzantium
// compileViaYul: also
// revertStrings: debug
// ----
// f(), 1 ether -> FAILURE, hex"08c379a0", 0x20, 34, "Ether sent to non-payable functi", "on"
// () -> FAILURE, hex"08c379a0", 0x20, 53, "Contract does not have fallback ", "nor receive functions"
