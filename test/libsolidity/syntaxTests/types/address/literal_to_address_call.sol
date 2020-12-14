contract C {
    function f() public returns (bool success) {
        (success, ) = (address(0)).call{value: 30}("");
    }
}
