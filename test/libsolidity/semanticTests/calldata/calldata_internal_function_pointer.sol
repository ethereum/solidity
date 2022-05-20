contract C {
    function(bytes calldata) returns (bytes1) x;
    constructor() { x = f; }
    function f(bytes calldata b) internal pure returns (bytes1) {
        return b[2];
    }
    function h(bytes calldata b) external returns (bytes1) {
        return x(b);
    }
    function g() external returns (bytes1) {
        bytes memory a = new bytes(34);
        a[2] = bytes1(uint8(7));
        return this.h(a);
    }
}
// ----
// g() -> 0x0700000000000000000000000000000000000000000000000000000000000000
