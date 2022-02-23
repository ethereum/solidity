==== Source: s1.sol ====


bytes constant a = b;
bytes constant b = hex"030102";

function fre() pure returns (bytes memory) {
    return a;
}

==== Source: s2.sol ====

import "s1.sol";

uint256 constant c = uint8(a[0]) + 2;

contract C {
    function f() public returns (bytes memory) {
        return a;
    }

    function g() public returns (bytes memory) {
        return b;
    }

    function h() public returns (uint) {
        return c;
    }

    function i() public returns (bytes memory) {
        return fre();
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 0x20, 3, "\x03\x01\x02"
// g() -> 0x20, 3, "\x03\x01\x02"
// h() -> 5
// i() -> 0x20, 3, "\x03\x01\x02"
