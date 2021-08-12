{
    {
        let _13_71 := 1
        let _17_72 := msize()
        let _22_75 := msize()
        let _25_76 := msize()
        let _30_80 := msize()
        let _32_81 := msize()
        // This should not be removed
        pop(keccak256(1, 2))
        let _104 := gt(not(_17_72), _13_71)
        let _105 := 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
        mstore(lt(or(gt(_13_71, or(or(gt(or(or(or(gt(or(gt(_105, _32_81), _13_71), _30_80), lt(or(_13_71, add(_25_76, _105)), _13_71)), _22_75), _13_71), _13_71), _104), _13_71)), _13_71), _13_71), _13_71)
        foo_singlereturn_1()
    }
    function foo_singlereturn_1()
    {
        extcodecopy(1, msize(), 1, 1)
    }
}
// ====
// EVMVersion: =homestead
// ----
// step: stackCompressor
//
// {
//     let _17_72 := msize()
//     let _22_75 := msize()
//     let _25_76 := msize()
//     let _30_80 := msize()
//     let _32_81 := msize()
//     pop(keccak256(1, 2))
//     let _105 := 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
//     mstore(lt(or(gt(1, or(or(gt(or(or(or(gt(or(gt(_105, _32_81), 1), _30_80), lt(or(1, add(_25_76, _105)), 1)), _22_75), 1), 1), gt(not(_17_72), 1)), 1)), 1), 1), 1)
//     foo_singlereturn_1()
//     function foo_singlereturn_1()
//     { extcodecopy(1, msize(), 1, 1) }
// }
