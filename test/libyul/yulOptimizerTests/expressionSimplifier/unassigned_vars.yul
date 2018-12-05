// c & d can't be optimized as expression simplifier doesn't handle default
// values yet
{
	let c
	let d
	let y := add(d, add(c, 7))
}
// ----
// expressionSimplifier
// {
//     let c
//     let d
//     let y := add(add(d, c), 7)
// }
