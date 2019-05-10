{
	for {} calldatasize() { mstore(1, 2) } {
		let x := 7
		mstore(4, 5)
		break
		revert(0, x)
	}
}
// ====
// step: controlFlowSimplifier
// ----
// {
//     for { } calldatasize() { mstore(1, 2) }
//     {
//         let x := 7
//         mstore(4, 5)
//         break
//         revert(0, x)
//     }
// }
