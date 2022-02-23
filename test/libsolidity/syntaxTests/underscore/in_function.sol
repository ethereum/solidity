contract C {
    function f() public pure returns (uint) {
        uint _;
        return _;
    }

    function g() public pure returns (uint) {
        uint _ = 1;
        return _;
    }

    function h() public pure {
        _;
    }
}
// ----
// DeclarationError 3726: (67-73): The name "_" is reserved.
// DeclarationError 3726: (154-160): The name "_" is reserved.
// DeclarationError 7576: (230-231): Undeclared identifier.
