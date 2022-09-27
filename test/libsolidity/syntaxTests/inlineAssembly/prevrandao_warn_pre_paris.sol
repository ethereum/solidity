function f() view returns (uint) {
    return block.prevrandao;
}
// ====
// EVMVersion: <paris
// ----
// Warning 9432: (43-59): "prevrandao" is not supported by the VM version and will be treated like "difficulty".
