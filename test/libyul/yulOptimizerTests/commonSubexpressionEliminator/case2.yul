{
    let _13 := 0x20
    let _14 := allocate(_13)
    pop(_14)
    let _15 := 2
    let _16 := 3
    let _17 := 0x40
    let _18 := allocate(_17)
    let _19 := array_index_access(_18, _16)
    mstore(_19, _15)
    function allocate(size) -> p
    {
        let _1 := 0x40
        let p_2 := mload(_1)
        p := p_2
        let _20 := add(p_2, size)
        mstore(_1, _20)
    }
    function array_index_access(array, index) -> p_1
    {
        let _21 := 0x20
        let _22 := mul(index, _21)
        p_1 := add(array, _22)
    }
}
// ====
// step: commonSubexpressionEliminator
// ----
// {
//     let _13 := 0x20
//     let _14 := allocate(_13)
//     pop(_14)
//     let _15 := 2
//     let _16 := 3
//     let _17 := 0x40
//     let _18 := allocate(_17)
//     let _19 := array_index_access(_18, _16)
//     mstore(_19, _15)
//     function allocate(size) -> p
//     {
//         let _1 := 0x40
//         let p_2 := mload(_1)
//         p := p_2
//         let _20 := add(p_2, size)
//         mstore(_1, _20)
//     }
//     function array_index_access(array, index) -> p_1
//     {
//         let _21 := 0x20
//         let _22 := mul(index, _21)
//         p_1 := add(array, _22)
//     }
// }
