// The first argument of div is not constant.
// keccak256 is not movable.
{
	sstore(0, msize())
	let a := div(keccak256(0, 0), 0)
	sstore(20, a)
}
// ----
// step: expressionSimplifier
//
// {
//     {
//         let _1 := msize()
//         let _2 := 0
//         sstore(_2, _1)
//         pop(keccak256(_2, _2))
//         sstore(20, 0)
//     }
// }
