contract A {
    uint x;
    function() public { x = 1; }
}
contract C is A {
    function() public { x = 2; }
}
