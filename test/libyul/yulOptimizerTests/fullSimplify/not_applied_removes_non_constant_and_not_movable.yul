// div is eliminated, but keccak256 has side-effects.
{
	let a := div(keccak256(0, 0), 0)
	mstore(0, a)
}
// ----
// fullSimplify
// {
//     let _1 := 0
//     pop(keccak256(_1, _1))
//     mstore(_1, 0)
// }
