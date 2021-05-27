// The yul code for the following contract
//  contract C {
//	  uint256[] x;
//	  function f() public { x[10] = 5; }
//  }

{
    let _1 := 0
    if eq(0x26121ff0, shr(224, calldataload(_1)))
    {
        if callvalue() { revert(_1, _1) }
        if slt(add(calldatasize(), not(3)), _1) { revert(_1, _1) }
        if iszero(lt(0x0a, sload(_1)))
        {
            mstore(_1, shl(224, 0x4e487b71))
            mstore(4, 0x32)
            revert(_1, 0x24)
        }
        mstore(_1, _1)
        // The hash should be evaluated here
        sstore(add(keccak256(_1, 0x20), 0x0a), 0x05)
    }
}
// ====
// EVMVersion: >=constantinople
// ----
// step: fullSuite
//
// {
//     {
//         let _1 := 0
//         if eq(0x26121ff0, shr(224, calldataload(_1)))
//         {
//             if callvalue() { revert(_1, _1) }
//             if slt(add(calldatasize(), not(3)), _1) { revert(_1, _1) }
//             if iszero(lt(0x0a, sload(_1)))
//             {
//                 mstore(_1, shl(224, 0x4e487b71))
//                 mstore(4, 0x32)
//                 revert(_1, 0x24)
//             }
//             sstore(0x290decd9548b62a8d60345a988386fc84ba6bc95484008f6362f93160ef3e56d, 0x05)
//         }
//     }
// }
