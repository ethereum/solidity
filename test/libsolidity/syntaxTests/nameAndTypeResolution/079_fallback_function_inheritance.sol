contract A {
    uint x;
    function() external { x = 1; }
}
contract C is A {
    function() external { x = 2; }
}
