contract c {
    uint result;

    function f(uint a, uint b) public {
        result += a + b;
    }

    function g(uint a) public {
        result *= a;
    }

    function test(uint a, bytes calldata data1, bytes calldata data2, uint b) external returns(uint r_a, uint r, uint r_b, uint l) {
        r_a = a;
        address(this).call(data1);
        address(this).call(data2);
        r = result;
        r_b = b;
        l = data1.length;
    }
}

// ----
// f(uint256,uint256): 8, 9 ->
// g(uint256): 3 ->
