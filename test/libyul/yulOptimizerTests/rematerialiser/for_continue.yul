{
	let a
	let b
	for { let i := 0 }
		lt(i, 10)
		{ i := add(a, b) } // `b` is always known to be caller() but `a` may be origin() or caller().
	{
		a := origin()
		b := origin()

		b := caller()
		if callvalue() { continue }
		a := caller()
	}
	mstore(a, b)
}
// ====
// step: rematerialiser
// ----
// {
//     let a
//     let b
//     for {
//         let i := 0
//     }
//     lt(i, 10)
//     {
//         i := add(a, caller())
//     }
//     {
//         a := origin()
//         b := origin()
//         b := caller()
//         if callvalue()
//         {
//             continue
//         }
//         a := caller()
//     }
//     mstore(a, b)
// }
