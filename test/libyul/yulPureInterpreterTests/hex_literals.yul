{
	let x := hex"112233445566778899aabbccddeeff6677889900"
	let y := hex"1234_abcd"
}
// ====
// printHex: true
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//   x = 0x112233445566778899aabbccddeeff6677889900000000000000000000000000
//   y = 0x1234abcd00000000000000000000000000000000000000000000000000000000
//
// Call trace:
