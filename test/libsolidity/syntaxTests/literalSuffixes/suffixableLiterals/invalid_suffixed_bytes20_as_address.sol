function suffix(address) pure suffix returns (address) {}

contract C {
    function f() public pure {
        hex"1234567890123456789012345678901234567890" suffix;
    }
}
// ----
// TypeError 8838: (111-156): The literal cannot be converted to type address accepted by the suffix function.
