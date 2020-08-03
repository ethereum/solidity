// Unassigned variables are assumed to be zero.
{
	let c
	let d
	let y := add(d, add(c, 7))
	sstore(8, y)
}
// ----
// step: expressionSimplifier
//
// { sstore(8, 7) }
