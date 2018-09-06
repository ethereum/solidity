contract c {
    modifier mod1(uint a) { if (msg.sender == address(a)) _; }
    modifier mod2 { if (msg.sender == address(2)) _; }
    function f() public mod1(7) mod2 { }
}
// ----
// Warning: (135-171): Function state mutability can be restricted to view
