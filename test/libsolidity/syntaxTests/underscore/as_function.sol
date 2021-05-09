contract C {
    function _() public pure {
    }

    function g() public pure {
        _();
    }

    function h() public pure {
        _;
    }
}
// ----
