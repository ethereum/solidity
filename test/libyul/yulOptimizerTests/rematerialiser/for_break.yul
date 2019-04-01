{
	let a
	let b
	for {let i := 0} lt(i, 10) {i := add(a, b)} {
		a := origin()
		b := origin()
		b := caller()
		// a=origin, b=caller
		if callvalue() { break }
		// a=origin, b=caller
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
//         i := add(caller(), caller())
//     }
//     {
//         a := origin()
//         b := origin()
//         b := caller()
//         if callvalue()
//         {
//             break
//         }
//         a := caller()
//     }
//     mstore(a, b)
// }
