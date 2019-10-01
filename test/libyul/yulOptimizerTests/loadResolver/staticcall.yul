{
    let a := 0
    let b := 1
    let c := 2
    sstore(a, b)
    mstore(900, 7)
    let d := staticcall(10000, 10, 0, 200, 0, 200)
    sstore(add(a, 1), mload(900))
    // Main test objective: replace this sload.
    mstore(0, sload(a))
}
// ====
// step: loadResolver
// EVMVersion: >=byzantium
// ----
// {
//     let a := 0
//     let b := 1
//     sstore(a, b)
//     let _1 := 7
//     let _2 := 900
//     mstore(_2, _1)
//     let _3 := 200
//     pop(staticcall(10000, 10, a, _3, a, _3))
//     sstore(1, mload(_2))
//     mstore(a, b)
// }
