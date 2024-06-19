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
//     {
//         let _1 := 20
//         let _2 := 10
//         sstore(_2, _1)
//         let _3 := _1
//         let _4 := 30
//         sstore(_4, _3)
//         verbatim_0i_0o("test")
//         sstore(_4, sload(_2))
//     }
// }
