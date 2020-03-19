{
	// Cost of rematerializating x is 1
	let x := 0xff
	// Reference to x is not rematerialized because the reference is in a loop
	for {} lt(x, 0x100) {}
	{
		let y := add(x, 1)
	}
}
// ----
// step: rematerialiser
//
// {
//     let x := 0xff
//     for { } lt(x, 0x100) { }
//     { let y := add(x, 1) }
// }
