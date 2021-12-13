// Unassigned variables are assumed to be zero.
{
	let c, d
	let y := add(d, add(c, 7))
	sstore(0, y)
}
// ----
// step: expressionSimplifier
//
// {
//     {
//         let c, d
//         sstore(d, 7)
//     }
// }
