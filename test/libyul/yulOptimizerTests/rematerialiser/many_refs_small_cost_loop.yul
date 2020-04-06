{
	// Cost of rematerializating x is 1
	let x := 0xff
	// Although x has low cost, it is not considered for
	// rematerialization because it is referenced more than 5 times
	for {} lt(x, 0x100) {}
	{
		let y := add(x, 1)
		let z := mul(x, 1)
		let a := div(x, 2)
		let b := mod(x, 3)
		let c := sdiv(x, 4)
	}
}
// ----
// step: rematerialiser
//
// {
//     let x := 0xff
//     for { } lt(x, 0x100) { }
//     {
//         let y := add(x, 1)
//         let z := mul(x, 1)
//         let a := div(x, 2)
//         let b := mod(x, 3)
//         let c := sdiv(x, 4)
//     }
// }
