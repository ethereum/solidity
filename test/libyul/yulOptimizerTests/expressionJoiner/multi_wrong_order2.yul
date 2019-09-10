{
	let a := mload(2)
	let b := mload(6)
	let x := mul(add(a, b), 2)
	sstore(x, 3)
}
// ====
// step: expressionJoiner
// ----
// {
//     let a := mload(2)
//     sstore(mul(add(a, mload(6)), 2), 3)
// }
