{
	let a
	let b
	let c
	let i := 0
	for {  }
		lt(i, 10)
		{ i := add(add(a, b), c) } // `b` is always known to be caller() but `a` and `c` may be origin() or caller().
	{
		a := origin()
		b := origin()
		c := origin()

		b := caller()
		if callvalue() { continue }
		a := caller()

		if callvalue() { continue }
		c := caller()
	}
	mstore(a, b)
}
// ====
// step: rematerialiser
// ----
// {
//     let a
//     let b
//     let c
//     let i := 0
//     for { } lt(i, 10) { i := add(add(a, caller()), c) }
//     {
//         a := origin()
//         b := origin()
//         c := origin()
//         b := caller()
//         if callvalue() { continue }
//         a := caller()
//         if callvalue() { continue }
//         c := caller()
//     }
//     mstore(a, b)
// }
