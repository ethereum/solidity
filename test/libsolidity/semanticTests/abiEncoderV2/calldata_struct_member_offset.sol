pragma experimental ABIEncoderV2;

contract C {
    struct A {
        uint256 a;
        uint256[] b;
    }
    struct B {
        A a;
        uint256 b;
    }
    function g(B calldata b) external pure returns(uint256) {
        return b.b;
    }
    function f() public view returns(uint256, uint256) {
        uint256[] memory arr = new uint256[](20);
        arr[0] = 31; arr[2] = 84;
        B memory b = B(A(420, arr), 11);
        return (b.b, this.g(b));
    }
}
// ----
// f() -> 11, 11
