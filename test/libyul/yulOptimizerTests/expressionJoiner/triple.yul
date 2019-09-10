{
	let a := mload(2)
	let b := mload(6)
	let c := mload(7)
	let x := mul(add(c, b), a)
	sstore(x, 3)
}
// ====
// step: expressionJoiner
// ----
// {
//     sstore(mul(add(mload(7), mload(6)), mload(2)), 3)
// }
