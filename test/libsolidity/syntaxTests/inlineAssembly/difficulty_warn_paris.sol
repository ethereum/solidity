function f() view returns (uint) {
    return block.difficulty;
}
// ====
// EVMVersion: >=paris
// ----
// Warning 8417: (46-62): "difficulty" was renamed and supplanted by "prevrandao" in the VM version paris.
