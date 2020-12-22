{
	mstore(64, 128)
	if callvalue() { revert(0, 0) }
	let _1 := 0
    // Does not invalidate location 64
	calldatacopy(128, _1, calldatasize())
	let z := mload(64)
	sstore(z, 1)
 }
// ----
// step: loadResolver
//
// {
//     let _2 := 128
//     mstore(64, _2)
//     if callvalue()
//     {
//         let _5 := 0
//         revert(_5, _5)
//     }
//     let _1 := 0
//     calldatacopy(_2, _1, calldatasize())
//     sstore(_2, 1)
// }
