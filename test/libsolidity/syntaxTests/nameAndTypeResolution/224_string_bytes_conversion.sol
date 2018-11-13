contract Test {
    string s;
    bytes b;
    function h(string calldata _s) pure external { bytes(_s).length; }
    function i(string memory _s) pure internal { bytes(_s).length; }
    function j() view internal { bytes(s).length; }
    function k(bytes calldata _b) pure external { string(_b); }
    function l(bytes memory _b) pure internal { string(_b); }
    function m() view internal { string(b); }
}
// ----
