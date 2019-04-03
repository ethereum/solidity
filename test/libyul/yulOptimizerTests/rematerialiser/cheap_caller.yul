{
    // The caller opcode is cheap, so inline it,
    // no matter how often it is used
    let a := caller()
    mstore(a, a)
    mstore(add(a, a), mload(a))
    sstore(a, sload(a))
}
// ====
// step: rematerialiser
// ----
// {
//     let a := caller()
//     mstore(caller(), caller())
//     mstore(add(caller(), caller()), mload(caller()))
//     sstore(caller(), sload(caller()))
// }
