{
	let a := mload(3)
	let b := mload(6)
	let x := mul(add(b, a), mload(2))
	sstore(x, 3)
}
// ----
// expressionJoiner
// {
//     let a := mload(3)
//     let b := mload(6)
//     sstore(mul(add(b, a), mload(2)), 3)
// }
