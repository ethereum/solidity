contract Test {
    string s;
    bytes b;
    function h(string _s) external { bytes(_s).length; }
    function i(string _s) internal { bytes(_s).length; }
    function j() internal { bytes(s).length; }
    function k(bytes _b) external { string(_b); }
    function l(bytes _b) internal { string(_b); }
    function m() internal { string(b); }
}
// ----
// Warning: (47-99): Function state mutability can be restricted to pure
// Warning: (104-156): Function state mutability can be restricted to pure
// Warning: (161-203): Function state mutability can be restricted to view
// Warning: (208-253): Function state mutability can be restricted to pure
// Warning: (258-303): Function state mutability can be restricted to pure
// Warning: (308-344): Function state mutability can be restricted to view
