{
	let a := mload(1)
    let a_1 := mload(0)
    a := a_1
    mstore(a_1, 0)
}
// ====
// step: ssaReverser
// ----
// {
//     let a := mload(1)
//     a := mload(0)
//     let a_1 := a
//     mstore(a_1, 0)
// }
