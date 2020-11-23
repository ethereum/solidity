contract C {
    function a(uint256 x) public returns (uint256) {
        return x + 1;
    }

    function b(uint256 x) public returns (uint256) {
        return x + 2;
    }

    function c(uint256 x) public returns (uint256) {
        return x + 3;
    }

    function d(uint256 x) public returns (uint256) {
        return x + 5;
    }

    function e(uint256 x) public returns (uint256) {
        return x + 8;
    }

    function test(uint256 x, uint256 i) public returns (uint256) {
        function(uint) internal returns (uint)[] memory arr =
            new function(uint) internal returns (uint)[](10);
        arr[0] = a;
        arr[1] = b;
        arr[2] = c;
        arr[3] = d;
        arr[4] = e;
        return arr[i](x);
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// test(uint256,uint256): 10, 0 -> 11
// test(uint256,uint256): 10, 1 -> 12
// test(uint256,uint256): 10, 2 -> 13
// test(uint256,uint256): 10, 3 -> 15
// test(uint256,uint256): 10, 4 -> 18
// test(uint256,uint256): 10, 5 -> FAILURE, hex"4e487b71", 0x51
