// The first argument of div is not constant.
// keccak256 is not movable.
{
	sstore(0, msize())
	let a := div(keccak256(0, 0), 0)
	sstore(20, a)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: expressionSimplifier
//
// {
//     {
//         sstore(0, msize())
//         pop(keccak256(0, 0))
//         sstore(20, 0)
//     }
// }
