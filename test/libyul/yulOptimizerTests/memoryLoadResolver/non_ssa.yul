// Test to see if non-ssa variables are encoded
{
    let x := 2
    x := calldataload(0)

    let y := calldataload(32)

    mstore(x, y)

    // Should not be resolved
    let a := mload(x)

    // Makes no sense
    let b := mload(y)
}
// ----
// step: memoryLoadResolver
//
// {
//     let x := 2
//     x := calldataload(0)
//     let y := calldataload(32)
//     mstore(x, y)
//     let a := mload(x)
//     let b := mload(y)
// }
