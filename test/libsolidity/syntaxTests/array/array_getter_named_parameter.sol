contract C {
    uint[] public myArray;

    function get(uint256 _index) public view returns (uint256) {
        return this.myArray({index: _index});
    }
}
// ----
// TypeError 4974: (121-150): Named argument "index" does not match function declaration.
