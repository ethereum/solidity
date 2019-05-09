contract C {
    function f(bool condition) public returns (uint x) {
        x = 23;
        if (condition)
            x = 42;
    }
    function g(bool condition) public returns (uint x) {
        x = 0;
        if (condition)
            x = 42;
        else
            x = 23;
    }
    function h(bool condition) public returns (uint x) {
        if (condition)
            return 42;
        x = 23;
    }
    function i(bool condition) public returns (uint x) {
        if (condition)
            x = 10;
        else
            return 23;
        x = 42;
    }
    function j(uint a, uint b) public returns (uint x, uint y) {
        x = 42;
        if (a + b < 10)
            x = a;
        else
            x = b;
        y = 100;
    }
    function k(uint a, uint b) public returns (uint x, uint y) {
        x = 42;
        do {
            if (a + b < 10)
            {
                if (a == b)
                {
                    x = 99; y = 99;
                    break;
                }
                else
                {
                    x = a;
                }
            }
            else
            {
                x = b;
                if (a != b)
                    y = 17;
                else
                    y = 13;
                break;
            }
            y = 100;
        } while(false);
    }
}
// ====
// compileViaYul: true
// ----
// f(bool): 0 -> 23
// f(bool): 1 -> 42
// g(bool): 0 -> 23
// g(bool): 1 -> 42
// h(bool): 0 -> 23
// h(bool): 1 -> 42
// i(bool): 0 -> 23
// i(bool): 1 -> 42
// j(uint256,uint256): 1, 3 -> 1, 100
// j(uint256,uint256): 3, 1 -> 3, 100
// j(uint256,uint256): 10, 23 -> 23, 100
// j(uint256,uint256): 23, 10 -> 10, 100
// k(uint256,uint256): 1, 3 -> 1, 100
// k(uint256,uint256): 3, 1 -> 3, 100
// k(uint256,uint256): 3, 3 -> 99, 99
// k(uint256,uint256): 10, 23 -> 23, 17
// k(uint256,uint256): 23, 10 -> 10, 17
// k(uint256,uint256): 23, 23 -> 23, 13
