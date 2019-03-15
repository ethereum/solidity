pragma experimental ABIEncoderV2;

contract C {
    function f(uint8[][1][] calldata s) external pure returns (bytes memory) {
        return msg.data;
    }
    function f2(uint256[][2][] calldata s) external pure returns (bytes memory) {
        return msg.data;
    }
    function reenc_f(uint8[][1][] calldata s) external view returns (bytes memory) {
        return this.f(s);
    }
    function reenc_f2(uint256[][2][] calldata s) external view returns (bytes memory) {
        return this.f2(s);
    }
    function g() external returns (bytes memory) {
        uint8[][1][] memory m = new uint8[][1][](1);
        m[0][0] = new uint8[](1);
        m[0][0][0] = 42;
        return this.f(m);
    }
    function h() external returns (bytes memory) {
        uint8[][1][] memory m = new uint8[][1][](1);
        m[0][0] = new uint8[](1);
        m[0][0][0] = 42;
        return this.reenc_f(m);
    }
    function i() external returns (bytes memory) {
        uint256[][2][] memory m = new uint256[][2][](1);
        m[0][0] = new uint256[](1);
        m[0][1] = new uint256[](1);
        m[0][0][0] = 42;
        m[0][1][0] = 42;
        return this.f2(m);
    }
    function j() external returns (bytes memory) {
        uint256[][2][] memory m = new uint256[][2][](1);
        m[0][0] = new uint256[](1);
        m[0][1] = new uint256[](1);
        m[0][0][0] = 42;
        m[0][1][0] = 42;
        return this.reenc_f2(m);
    }
}
// ====
// EVMVersion: >homestead
// ----
// g() -> 32, 196, hex"eccb829a", 32, 1, 32, 32, 1, 42, hex"00000000000000000000000000000000000000000000000000000000"
// h() -> 32, 196, hex"eccb829a", 32, 1, 32, 32, 1, 42, hex"00000000000000000000000000000000000000000000000000000000"
