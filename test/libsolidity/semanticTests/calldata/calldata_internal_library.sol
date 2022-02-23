library L {
    function f(uint, bytes calldata _x, uint) internal returns (bytes1) {
        return _x[2];
    }
}
contract C {
    function f(bytes calldata a)
        external
        returns (bytes1)
    {
        return L.f(3, a, 9);
    }
    function g() public returns (bytes1) {
        bytes memory x = new bytes(4);
        x[2] = 0x08;
        return this.f(x);
    }
}
// ====
// compileViaYul: also
// ----
// g() -> 0x0800000000000000000000000000000000000000000000000000000000000000
