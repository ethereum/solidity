{
    mstore(10, 20)
    // cannot be resolved because of verbatim
    sstore(0, mload(10))
    verbatim_0i_0o("test")
}
// ----
// step: loadResolver
//
// {
//     let _1 := 20
//     let _2 := 10
//     mstore(_2, _1)
//     sstore(0, mload(_2))
//     verbatim_0i_0o("test")
// }
