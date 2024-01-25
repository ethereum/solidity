{
    tstore(0, 13)
    tstore(0, 42)
    tstore(0x20, tload(0))
    let x:= tload(0x20)
    tstore(0, x)
    pop(tload(0x20))
}
// ====
// EVMVersion: >=cancun
// ----
// step: fullSuite
//
// {
//     {
//         tstore(0, 13)
//         tstore(0, 42)
//         tstore(0x20, tload(0))
//         tstore(0, tload(0x20))
//     }
// }
