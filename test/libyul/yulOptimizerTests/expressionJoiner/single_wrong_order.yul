{
	let a := mload(3)
	let b := sload(a)
	let c := mload(7)
	let d := add(b, c)
	sstore(d, 0)
}
// ====
// step: expressionJoiner
// ----
// {
//     let b := sload(mload(3))
//     sstore(add(b, mload(7)), 0)
// }
