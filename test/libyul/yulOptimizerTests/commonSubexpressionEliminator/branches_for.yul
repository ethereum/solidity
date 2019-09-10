{
	let a := 1
	let b := codesize()
	for { } lt(1, codesize()) { mstore(1, codesize()) a := add(a, codesize()) } {
		mstore(1, codesize())
	}
	mstore(1, codesize())
}
// ====
// step: commonSubexpressionEliminator
// ----
// {
//     let a := 1
//     let b := codesize()
//     for { }
//     lt(1, b)
//     {
//         mstore(1, b)
//         a := add(a, b)
//     }
//     { mstore(1, b) }
//     mstore(1, b)
// }
