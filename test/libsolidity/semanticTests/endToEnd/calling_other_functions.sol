contract collatz {
    function run(uint x) public returns(uint y) {
        while ((y = x) > 1) {
            if (x % 2 == 0) x = evenStep(x);
            else x = oddStep(x);
        }
    }

    function evenStep(uint x) public returns(uint y) {
        return x / 2;
    }

    function oddStep(uint x) public returns(uint y) {
        return 3 * x + 1;
    }
}

// ----
// run(uint256): 0x0 -> 0x0
// run(uint256): 0x1 -> 0x1
// run(uint256): 0x2 -> 0x1
// run(uint256): 0x8 -> 0x1
// run(uint256): 0x7f -> 0x1
