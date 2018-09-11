contract C {
    mapping(uint => address payable) m;
    mapping(uint => address payable[]) n;
    function f() public view {
        address payable a;
        address payable[] memory b;
        mapping(uint => address payable) storage c = m;
        mapping(uint => address payable[]) storage d = n;
        a; b; c; d;
    }
}
