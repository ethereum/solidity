abstract contract A {
    function f() public view mod {
        require(block.timestamp > 10);
    }
    modifier mod() virtual;
}
