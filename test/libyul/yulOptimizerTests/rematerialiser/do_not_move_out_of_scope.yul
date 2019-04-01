// Cannot replace `let b := x` by `let b := a` since a is out of scope.
{
	let x
	{
		let a := sload(0)
		x := a
	}
	let b := x
}
// ====
// step: rematerialiser
// ----
// {
//     let x
//     {
//         let a := sload(0)
//         x := a
//     }
//     let b := x
// }
