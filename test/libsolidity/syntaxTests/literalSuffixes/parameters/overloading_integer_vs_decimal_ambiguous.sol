function suffix(uint) pure returns (int) {}
function suffix(uint, uint) pure returns (int) {}

contract C {
    function f() public pure {
        int a = 1 suffix;
    }
}
// ----
// DeclarationError 7920: (157-163): Identifier not found or not unique.
