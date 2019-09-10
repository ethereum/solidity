{
	// This is not joined because a is referenced multiple times
	let a := mload(2)
	let b := mload(a)
	a := 4
}
// ====
// step: expressionJoiner
// ----
// {
//     let a := mload(2)
//     let b := mload(a)
//     a := 4
// }
