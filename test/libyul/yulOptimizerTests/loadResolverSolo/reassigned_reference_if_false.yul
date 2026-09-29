{
    let dummyValue := 0
    let dummyOffset := 0
    mstore(dummyOffset, dummyValue)
    let v := 32
    if false { v := 99 }
    let a1 := v
    let a := a1
    let oldValue := 67
    mstore(a, oldValue) // stores 67 at offset 32
    if true { v := 0 }
    let b := add(v, 32)
    let newValue := 7
    mstore(b, newValue) // stores 7 at offset 32
    let result := mload(a) // loads value at offset 32
}
// ----
// step: loadResolverSolo
//
// {
//     let dummyValue := 0
//     let dummyOffset := 0
//     mstore(dummyOffset, dummyValue)
//     let v := 32
//     if false { v := 99 }
//     let a1 := v
//     let a := a1
//     let oldValue := 67
//     mstore(a, oldValue)
//     if true { v := 0 }
//     let b := add(v, 32)
//     let newValue := 7
//     mstore(b, newValue)
//     let result := oldValue
// }
