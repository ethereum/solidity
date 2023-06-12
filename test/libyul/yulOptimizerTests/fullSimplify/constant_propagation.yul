{
    let a := add(7, sub(mload(0), 7))
    mstore(a, 0)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: fullSimplify
//
// { { mstore(mload(0), 0) } }
