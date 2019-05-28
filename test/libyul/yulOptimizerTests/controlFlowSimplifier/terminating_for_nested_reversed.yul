{
    for {} calldatasize() { mstore(8, 9) } {
        for {} calldatasize() { mstore(1, 2) } {
            mstore(4, 5)
            continue
        }
        break
    }
}
// ====
// step: controlFlowSimplifier
// ----
// {
//     if calldatasize()
//     {
//         for { } calldatasize() { mstore(1, 2) }
//         {
//             mstore(4, 5)
//             continue
//         }
//     }
// }
