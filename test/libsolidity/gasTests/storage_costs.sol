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
//   codeDepositCost: 31400
//   executionCost: 88
//   totalCost: 31488
// external:
//   readX(): 2285
//   resetX(): 5111
//   setX(uint256): 22307
