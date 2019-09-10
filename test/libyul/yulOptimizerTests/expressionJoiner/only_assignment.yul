{
	// This is not joined because a is referenced multiple times
	function f(a) -> x {
		a := mload(2)
		x := add(a, 3)
	}
}
// ====
// step: expressionJoiner
// ----
// {
//     function f(a) -> x
//     {
//         a := mload(2)
//         x := add(a, 3)
//     }
// }
