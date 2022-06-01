pragma abicoder v2;

contract C {
    function f() public returns (uint8[] memory) {
        return [1, 2, 3, 4, 5];
    }

    function g() public returns (uint8[][] memory) {
        return [[1, 2], [3], []];
    }

    function h() public returns (uint8[][3] memory) {
        return [[1, 2], [3], []];
    }

    function i() public returns (uint8[2][] memory) {
        return [[1, 2], [3,4], [5,6]];
    }
}

// ====
// compileViaYul: true
// ----
// f() -> 0x20, 5, 1, 2, 3, 4, 5
// g() -> 0x20, 3, 0x60, 0xc0, 0x0100, 2, 1, 2, 1, 3, 0
// h() -> 0x20, 0x60, 0xc0, 0x0100, 2, 1, 2, 1, 3, 0
// i() -> 0x20, 3, 1, 2, 3, 4, 5, 6
