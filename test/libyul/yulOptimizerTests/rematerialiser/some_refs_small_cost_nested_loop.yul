{
	// Cost of rematerializating x is 1
	let x := 0xff
	// Although x has a low cost and fewer than 6 references,
	// its references in a loop are not rematerialized
	for {} lt(x, 0x100) {}
	{
		let y := add(x, 1)
		for {} lt(x, 0x200) {}
		{
			let z := mul(x, 2)
		}
	}
}
// ====
// step: rematerialiser
// ----
// {
//     let x := 0xff
//     for { } lt(x, 0x100) { }
//     {
//         let y := add(x, 1)
//         for { } lt(x, 0x200) { }
//         { let z := mul(x, 2) }
//     }
// }
