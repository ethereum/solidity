{
	let x := mload(0)
	x := 0
	mstore(0, add(7, x))
}
// ----
// step: expressionSimplifier
//
// {
//     let _1 := 0
//     let x := mload(_1)
//     x := _1
//     mstore(_1, 7)
// }
