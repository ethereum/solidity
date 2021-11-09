{
    function recursive() { recursive() }
    function terminating() { stop() }
    function maybeReverting() { if calldataload(0) { revert(0, 0) } }

    let a := calldataload(7)
    if a { recursive() }
    a := 0

    a := calldataload(a)
    if a { maybeReverting() }

    a := calldataload(a)
    if a { terminating() }
    a := 0

    sstore(0, a)
}
// ----
// step: conditionalUnsimplifier
//
// {
//     function recursive()
//     { recursive() }
//     function terminating()
//     { stop() }
//     function maybeReverting()
//     {
//         if calldataload(0) { revert(0, 0) }
//     }
//     let a := calldataload(7)
//     if a { recursive() }
//     a := calldataload(a)
//     if a { maybeReverting() }
//     a := calldataload(a)
//     if a { terminating() }
//     sstore(0, a)
// }
