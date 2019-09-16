contract A {
    uint x;
    function() external { x = 1; }
}
contract C is A {
    function() override external { x = 2; }
}
