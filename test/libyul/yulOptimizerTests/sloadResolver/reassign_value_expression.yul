{
    let x := calldataload(1)
    let a := add(x, 10)
    sstore(a, 7)
    // This clears the expression assigned to ``a`` but
    // should not clear storage knowledge
    x := 9
    mstore(sload(a), 11)
    // This, on the other hand, actually clears knowledge
    a := 33
    mstore(sload(a), 11)
    // Try again with different expression to avoid
    // clearing because we cannot know if it is different
    a := 39
    mstore(sload(a), 11)
}
// ====
// step: sloadResolver
// ----
// {
//     let x := calldataload(1)
//     let a := add(x, 10)
//     let _3 := 7
//     sstore(a, _3)
//     x := 9
//     let _4 := 11
//     mstore(_3, _4)
//     a := 33
//     mstore(sload(a), _4)
//     a := 39
//     mstore(sload(a), _4)
// }
