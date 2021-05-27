contract C {
	function f() public {}
}
// ====
// EVMVersion: >=byzantium
// compileViaYul: also
// revertStrings: debug
// ----
// f(), 1 ether -> FAILURE, hex"4e487b71", 0x02
// () -> FAILURE, hex"08c379a0", 0x20, 53, "Contract does not have fallback ", "nor receive functions"
