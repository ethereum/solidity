contract C {
    function f(uint256 x, uint256 y) public pure returns (uint256 z){
        assembly { z := add(x, y) }
    }
}
// ----
// :C(creation) true
// :C(runtime) true
