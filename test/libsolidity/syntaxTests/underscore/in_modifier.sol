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
