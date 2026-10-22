contract A { constructor() { revert("A"); } }
contract B is A { constructor() { revert("B"); } }
contract C1 is B layout at 2**256 - 2**42 { constructor() { revert("C"); } }
contract C2 is B { constructor() { revert("C"); } }
contract F {
    function withSpecifier() public returns (string memory) {
        new C1();
    }
    function withoutSpecifier() public returns (string memory) {
        new C2();
    }
}
// ====
// targetContract: F
// ----
// withSpecifier() -> FAILURE, hex"08c379a0", 0x20, 1, "A"
// withoutSpecifier() -> FAILURE, hex"08c379a0", 0x20, 1, "A"
