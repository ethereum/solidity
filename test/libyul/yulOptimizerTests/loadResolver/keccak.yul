{
    mstore(0, 10)
    /// >>> int.from_bytes(web3.Web3.keccak(int.to_bytes(10, 32, byteorder='big')), byteorder='big')
    /// 89717814153306320011181716697424560163256864414616650038987186496166826726056
    let val := keccak256(0, 32)
    sstore(0, val)
}
// ----
// step: loadResolver
//
// {
//     let _1 := 10
//     let _2 := 0
//     mstore(_2, _1)
//     sstore(_2, 89717814153306320011181716697424560163256864414616650038987186496166826726056)
// }
