contract C {
    function f() public pure returns (uint32 y) {
        uint8 x = uint8(uint256(0x31313131313131313131313131313131));
        assembly { y := x }
    }

    function g() public pure returns (bytes32 y) {
        bytes1 x = bytes1(bytes16(0x31313131313131313131313131313131));
        assembly { y := x }
    }

    function h() external returns (bytes32 y) {
        bytes1 x;
        assembly { x := sub(0,1) }
        y = x;
    }
}
// ====
// compileViaYul: true
// ----
// f() -> 0x31
// g() -> 0x3100000000000000000000000000000000000000000000000000000000000000
// h() -> 0xff00000000000000000000000000000000000000000000000000000000000000
