// Unassigned variables are assumed to be zero.
{
	let c, d
	let y := add(d, add(c, 7))
}
// ====
// step: expressionSimplifier
// ----
// {
//     let c, d
//     let y := 7
// }
