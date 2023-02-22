contract test {
    function f(uint x) public returns(uint d) {
        return x > 100 ?
                    x > 1000 ? 1000 : 100
                    :
                    x > 50 ? 50 : 10;
    }
}
// ----
// f(uint256): 1001 -> 1000
// f(uint256): 500 -> 100
// f(uint256): 80 -> 50
// f(uint256): 40 -> 10
