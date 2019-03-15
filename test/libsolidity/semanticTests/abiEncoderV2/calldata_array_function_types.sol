pragma experimental ABIEncoderV2;

contract C {
    function f(function() external returns (uint)[] calldata s) external returns (uint, uint, uint) {
        assert(s.length == 3);
        return (s[0](), s[1](), s[2]());
    }
    function f_reenc(function() external returns (uint)[] calldata s) external returns (uint, uint, uint) {
        return this.f(s);
    }
    function getter1() external returns (uint) {
        return 23;
    }
    function getter2() external returns (uint) {
        return 37;
    }
    function getter3() external returns (uint) {
        return 71;
    }
    function g(bool reenc) external returns (uint, uint, uint) {
        function() external returns (uint)[] memory a = new function() external returns (uint)[](3);
        a[0] = this.getter1;
        a[1] = this.getter2;
        a[2] = this.getter3;
        return reenc ? this.f_reenc(a) : this.f(a);
    }
}
// ----
// g(bool): false -> 23, 37, 71
// g(bool): true -> 23, 37, 71
