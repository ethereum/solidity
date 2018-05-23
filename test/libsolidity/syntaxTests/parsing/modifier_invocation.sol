contract c {
    modifier mod1(uint a) { if (msg.sender == address(a)) _; }
    modifier mod2 { if (msg.sender == address(2)) _; }
    function f() mod1(7) mod2 { }
}
// ----
// Warning: (135-164): No visibility specified. Defaulting to "public". 
// Warning: (135-164): Function state mutability can be restricted to view
