{
    // Constants cost depending on their magnitude.
    // Rematerialize small constants only if they are
    // not used too often.
    // b is used 5 times
    let b := 2
    mstore(b, b)
    mstore(add(b, b), b)
    // a is used 7 times
    let a := 1
    mstore(a, a)
    mstore(add(a, a), a)
    mstore(a, mload(a))
}
// ====
// step: rematerialiser
// ----
// {
//     let b := 2
//     mstore(2, 2)
//     mstore(add(2, 2), 2)
//     let a := 1
//     mstore(a, a)
//     mstore(add(a, a), a)
//     mstore(a, mload(a))
// }
