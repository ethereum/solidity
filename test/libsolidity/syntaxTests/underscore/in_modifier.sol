contract C {
    modifier m() {
        _;
    }

    modifier n() {
        string memory _ = "";
        _;
        revert(_);
    }

    function f() m() public {
    }

    function g() n() public {
    }
}
// ----
// DeclarationError 3726: (77-92): The name "_" is reserved.
