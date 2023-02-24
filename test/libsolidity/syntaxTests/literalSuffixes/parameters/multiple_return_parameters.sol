struct S { uint x; }

function uintUintSuffix(uint) pure suffix returns (uint, uint) {
    return (1, 2);
}

function bytesStructContractSuffix(string memory s) pure suffix returns (bytes memory, S memory, C) {
    return (bytes(s), S(42), C(address(0)));
}

contract C {
    function f() public pure returns (uint, uint) {
        return 1 uintUintSuffix;
    }

    function g() public pure returns (bytes memory, S memory, C) {
        return "abcd" bytesStructContractSuffix;
    }
}
// ----
// TypeError 7848: (72-84): Literal suffix functions must return exactly one value.
// TypeError 7848: (181-208): Literal suffix functions must return exactly one value.
