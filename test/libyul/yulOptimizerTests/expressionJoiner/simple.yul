{
	let a := mload(2)
	let x := calldataload(a)
	sstore(x, 3)
}
// ====
// step: expressionJoiner
// ----
// {
//     sstore(calldataload(mload(2)), 3)
// }
