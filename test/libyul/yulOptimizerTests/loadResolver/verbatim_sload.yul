{
    sstore(10, 20)
    // will be resolved
    sstore(30, sload(10))
    verbatim_0i_0o("test")
    // will not be resolved
    sstore(30, sload(10))
}
// ----
// step: loadResolver
//
// {
//     let _1 := 20
//     let _2 := 10
//     sstore(_2, _1)
//     let _4 := _1
//     let _5 := 30
//     sstore(_5, _4)
//     verbatim_0i_0o("test")
//     sstore(_5, sload(_2))
// }
