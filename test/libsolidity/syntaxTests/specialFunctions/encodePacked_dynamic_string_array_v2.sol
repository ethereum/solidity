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
// TypeError 9578: (112-113='m'): Type not supported in packed mode.
// TypeError 9578: (178-179='s'): Type not supported in packed mode.
