contract Test {
    string s;
    bytes b;
    function h(string _s) external { bytes(_s).length; }
    function i(string memory _s) internal { bytes(_s).length; }
    function j() internal { bytes(s).length; }
    function k(bytes _b) external { string(_b); }
    function l(bytes memory _b) internal { string(_b); }
    function m() internal { string(b); }
}
// ----
// Warning: (47-99): Function state mutability can be restricted to pure
// Warning: (104-163): Function state mutability can be restricted to pure
// Warning: (168-210): Function state mutability can be restricted to view
// Warning: (215-260): Function state mutability can be restricted to pure
// Warning: (265-317): Function state mutability can be restricted to pure
// Warning: (322-358): Function state mutability can be restricted to view
