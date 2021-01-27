{

    let x := 1
    let y := 2
    let z := 3

    mstore(x, y)

    let a := calldataload(0)

    if a
    {
        // resolves to y
        let tmp_1 := mload(x)
        mstore(x, z)
        // resolves to z
        let tmp_2 := mload(x)
    }

    // should not be able to resolve x
    let value := mload(x)
}
// ----
// step: memoryLoadResolver
//
// {
//     let x := 1
//     let y := 2
//     let z := 3
//     mstore(x, y)
//     let a := calldataload(0)
//     if a
//     {
//         let tmp_1 := y
//         mstore(x, z)
//         let tmp_2 := z
//     }
//     let value := mload(x)
// }
