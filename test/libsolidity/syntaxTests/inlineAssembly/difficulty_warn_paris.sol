function f() view returns (uint) {
    return block.difficulty;
}
// ====
// EVMVersion: >=paris
// ----
// Warning 8417: (43-59): "difficulty" was renamed and supplanted by "prevrandao" in the VM version paris.
