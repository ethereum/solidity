contract A {
	receive () external payable {}
}
// ====
// EVMVersion: >=byzantium
// compileViaYul: also
// revertStrings: debug
// ----
// (): hex"00" -> FAILURE, hex"08c379a0", 0x20, 41, "Unknown signature and no fallbac", "k defined"
