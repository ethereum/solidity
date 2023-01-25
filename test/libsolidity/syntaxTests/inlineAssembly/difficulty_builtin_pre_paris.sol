contract C {
    function f() public view returns (uint256) {
        return block.difficulty;
    }

    function g() public view returns (uint256 ret) {
        assembly {
            ret := difficulty()
        }
    }
}
// ====
// EVMVersion: <paris
// ----
