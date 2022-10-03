function f() view returns (uint) {
    return block.prevrandao;
}
// ====
// EVMVersion: <paris
// ----
// Warning 9432: (46-62): "prevrandao" is not supported by the VM version and will be treated like "difficulty".
