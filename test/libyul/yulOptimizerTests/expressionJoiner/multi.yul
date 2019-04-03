{
	let a := mload(2)
	let b := mload(6)
	let x := mul(add(b, a), 2)
	sstore(x, 3)
}
// ====
// step: expressionJoiner
// ----
// {
//     sstore(mul(add(mload(6), mload(2)), 2), 3)
// }
