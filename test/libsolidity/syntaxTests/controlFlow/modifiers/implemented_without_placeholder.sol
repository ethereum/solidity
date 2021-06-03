abstract contract A {
    function f() public view mod {
        require(block.timestamp > 10);
    }
    modifier mod() virtual { }
}
// ----
// SyntaxError 2883: (129-132): Modifier body does not contain '_'.
