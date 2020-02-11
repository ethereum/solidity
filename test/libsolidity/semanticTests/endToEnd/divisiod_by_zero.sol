contract C {
    function div(uint a, uint b) public returns(uint) {
        return a / b;
    }

    function mod(uint a, uint b) public returns(uint) {
        return a % b;
    }
}

// ----
// div(uint256,uint256): 7, 2 -> 3
// div(uint256,uint256):"7, 2" -> "3"
// div(uint256,uint256): 7, 0 -> 
// div(uint256,uint256):"7, 0" -> ""
// mod(uint256,uint256): 7, 2 -> 1
// mod(uint256,uint256):"7, 2" -> "1"
// mod(uint256,uint256): 7, 0 -> 
// mod(uint256,uint256):"7, 0" -> ""
