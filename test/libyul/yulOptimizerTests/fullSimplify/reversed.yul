{
    let a := add(0, mload(0))
    mstore(0, a)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: fullSimplify
//
// { { mstore(0, mload(0)) } }
