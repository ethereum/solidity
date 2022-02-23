contract C {
    string[] s;
    function f() public pure {
        string[] memory m;
        abi.encodePacked(m);
    }
    function g() public pure {
        abi.encodePacked(s);
    }
}
// ----
// TypeError 9578: (112-113): Type not supported in packed mode.
// TypeError 9578: (178-179): Type not supported in packed mode.
