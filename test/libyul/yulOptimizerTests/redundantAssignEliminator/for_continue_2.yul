{
	// Cannot be removed, because we might run the loop only once
	let x := 1
	for { } calldataload(0) { }
	{
		if callvalue() {
			x := 2 // is preserved because of continue stmt below.
			continue
		}
		x := 3
	}
	mstore(x, 0x42)
}
// ----
// redundantAssignEliminator
// {
//     let x := 1
//     for {
//     }
//     calldataload(0)
//     {
//     }
//     {
//         if callvalue()
//         {
//             x := 2
//             continue
//         }
//         x := 3
//     }
//     mstore(x, 0x42)
// }
