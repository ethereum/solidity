// The first argument of div is not constant.
// keccak256 is not movable.
{
	let a := div(keccak256(0, 0), 0)
}
// ====
// step: expressionSimplifier
// ----
// {
//     let a := div(keccak256(0, 0), 0)
// }
