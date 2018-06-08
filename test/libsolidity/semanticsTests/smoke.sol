contract C {
    function f(uint256 a) public returns(uint256, uint256) {
        return (a, a);
    }
    function g() public returns(uint256) {
        return 42000;
    }
    function g2() public returns(uint256) {
        return 0x42001;
    }
    function h() public returns(uint256) {
        revert();
    }
    function i() public returns(bytes32) {
        return bytes32("abc");
    }
    function j() public returns(bool) {
        return true;
    }
    function k() public returns(bytes32) {
        return bytes2(0x1234);
    }
}
// ----
// f(uint256): 21
// -> 21, 21
// g()
// -> 42000
// g2()
// -> 0x42001
// h()
// REVERT
// non_existing()
// REVERT
// i()
// -> "abc"
// j()
// -> true
// k()
// -> hex"1234"
