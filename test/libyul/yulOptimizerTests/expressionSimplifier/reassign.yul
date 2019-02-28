{
	let x := mload(0)
	x := 0
	mstore(0, add(7, x))
}
// ----
// expressionSimplifier
// {
//     let x := mload(0)
//     x := 0
//     mstore(0, 7)
// }
