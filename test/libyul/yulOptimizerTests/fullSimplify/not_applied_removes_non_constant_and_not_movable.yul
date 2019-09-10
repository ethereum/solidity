// div is eliminated, but create has side-effects.
{
	let a := div(create(0, 0, 0), 0)
	mstore(0, a)
}
// ====
// step: fullSimplify
// ----
// {
//     let _1 := 0
//     pop(create(_1, _1, _1))
//     mstore(_1, 0)
// }
