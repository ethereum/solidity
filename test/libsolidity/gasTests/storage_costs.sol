contract C {
    uint x;
    function setX(uint y) public {
        x = y;
    }
    function resetX() public {
        x = 0;
    }
    function readX() public view returns(uint) {
        return x;
    }
}
// ====
// optimize: true
// optimize-yul: true
// ----
// creation:
//   codeDepositCost: 26600
//   executionCost: 81
//   totalCost: 26681
// external:
//   readX(): 2290
//   resetX(): 5116
//   setX(uint256): 22301
