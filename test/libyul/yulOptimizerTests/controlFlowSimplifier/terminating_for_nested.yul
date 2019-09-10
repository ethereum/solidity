{
    for {} calldatasize() { mstore(8, 9) } {
        for {} calldatasize() { mstore(1, 2) } {
            mstore(4, 5)
            break
        }
        if mload(10) { continue }
        break
    }
}
// ====
// step: controlFlowSimplifier
// ----
// {
//     for { } calldatasize() { mstore(8, 9) }
//     {
//         if calldatasize() { mstore(4, 5) }
//         if mload(10) { continue }
//         break
//     }
// }
