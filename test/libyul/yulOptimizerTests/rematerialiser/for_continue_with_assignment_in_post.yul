{
	let a
	let b
	let c
	for {
		let i := 0
		b := origin()
		c := origin()
	}
	lt(i, 10)
	{
		i := add(a, b)
		b := callvalue()
		c := caller()
	}
	{
		a := origin()

		b := caller()
		if callvalue() { continue }
		a := caller()
	}
	let x := b // does not rematerialize as b may be either origin() or callvalue() (btw: not caller())
	let y := c // does not rematerialize as c may be either origin() or caller()
}
// ====
// step: rematerialiser
// ----
// {
//     let a
//     let b
//     let c
//     for {
//         let i := 0
//         b := origin()
//         c := origin()
//     }
//     lt(i, 10)
//     {
//         i := add(a, caller())
//         b := callvalue()
//         c := caller()
//     }
//     {
//         a := origin()
//         b := caller()
//         if callvalue()
//         {
//             continue
//         }
//         a := caller()
//     }
//     let x := b
//     let y := c
// }
