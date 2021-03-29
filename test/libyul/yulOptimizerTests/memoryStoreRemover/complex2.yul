{
    f()
    let z := 100
    pop(mload(z))

    if calldataload(0) {
        // does not read from location [20, 52)
        pop(keccak256(z, z))
    }

    function f() {
        let x := 10
        let y := 20
        // can be removed
        mstore(x, y)
    }
}
// ----
// step: memoryStoreRemover
//
// {
//     f()
//     let z := 100
//     pop(mload(z))
//     if calldataload(0) { pop(keccak256(z, z)) }
//     function f()
//     {
//         let x := 10
//         let y := 20
//         pop(y)
//     }
// }
