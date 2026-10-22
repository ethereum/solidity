contract A {
    modifier m() virtual {
        revert("A");
        _;
    }
}
contract B is A {
    modifier m() virtual override {
        revert("B");
        _;
    }
}
contract C1 is B layout at 2**256 - 2**42 {
    function f() public m {}
    modifier m() virtual override {
        revert("C");
        _;
    }
}
contract C2 is B {
    function f() public m {}
    modifier m() virtual override {
        revert("C");
        _;
    }
}

contract F {
    function withSpecifier() public returns (string memory) {
        new C1().f();
    }
    function withoutSpecifier() public returns (string memory) {
        new C2().f();
    }
}
// ====
// targetContract: F
// ----
// withSpecifier() -> FAILURE, hex"08c379a0", 0x20, 1, "C"
// withoutSpecifier() -> FAILURE, hex"08c379a0", 0x20, 1, "C"
