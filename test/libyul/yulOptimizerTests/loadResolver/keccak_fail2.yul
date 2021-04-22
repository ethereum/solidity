{
    mstore(100, 10)
    mstore(132, 5)
    // Won't be evaluated, even though the value is known
    let val := keccak256(10, 33)
    sstore(0, val)
}
// ----
// step: loadResolver
//
// {
//     let _1 := 10
//     mstore(100, _1)
//     mstore(132, 5)
//     sstore(0, keccak256(_1, 33))
// }
