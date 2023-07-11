contract C {
    function f() public view returns (uint256) {
        return block.prevrandao;
    }
}
// ====
// EVMVersion: <paris
// ----
// Warning 9432: (77-93): "prevrandao" is not supported by the VM version and will be treated as "difficulty".
