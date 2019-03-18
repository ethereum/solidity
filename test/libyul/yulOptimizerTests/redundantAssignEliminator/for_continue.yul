{
	// Cannot be removed, because we might run the loop only once
	let x := 1
	for { } calldataload(0) { }
	{
		x := 2 // Will not be removed as if-condition can be false.
		if callvalue() {
			x := 3
			continue
		}
		mstore(x, 2)
	}
	x := 3
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
//         x := 2
//         if callvalue()
//         {
//             x := 3
//             continue
//         }
//         mstore(x, 2)
//     }
// }
