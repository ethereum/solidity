contract C {
    function(bytes calldata) returns (byte) x;
    constructor() { x = f; }
    function f(bytes calldata b) internal pure returns (byte) {
        return b[2];
    }
    function h(bytes calldata b) external returns (byte) {
        return x(b);
    }
    function g() external returns (byte) {
        bytes memory a = new bytes(34);
        a[2] = byte(uint8(7));
        return this.h(a);
    }
}
// ----
// g() -> 0x0700000000000000000000000000000000000000000000000000000000000000
