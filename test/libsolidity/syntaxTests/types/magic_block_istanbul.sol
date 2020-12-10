contract C {
    function f() public view returns (address payable) {
        return block.coinbase;
    }
    function g() public view returns (uint) {
        return block.difficulty;
    }
    function h() public view returns (uint) {
        return block.gaslimit;
    }
    function i() public view returns (uint) {
        return block.timestamp;
    }
    function j() public view returns (uint) {
        return block.chainid;
    }
}
// ====
// EVMVersion: >=istanbul
