{
	// We have an interleaved "add" here, so we cannot inline "a"
	// (note that this component does not analyze whether
	// functions are pure or not)
	let a := mload(2)
	let b := mload(6)
	let x := mul(a, add(2, b))
	sstore(x, 3)
}
// ====
// step: expressionJoiner
// ----
// {
//     let a := mload(2)
//     sstore(mul(a, add(2, mload(6))), 3)
// }
