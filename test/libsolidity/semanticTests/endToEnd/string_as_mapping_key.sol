contract Test {
    mapping(string => uint) data;

    function set(string memory _s, uint _v) public {
        data[_s] = _v;
    }

    function get(string memory _s) public returns(uint) {
        return data[_s];
    }
}

// ====
// compileViaYul: also
// ----
// set(string,uint256): 64, 7, 13, "Hello, World!" ->
// set(string,uint256): 64, 9, 6, "string" ->
// set(string,uint256): 64, 10, 1, "1" ->
// get(string): 32, 13, "Hello, World!" -> 7
// get(string): 32, 6, "string" -> 9
// get(string): 32, 1, "1" -> 10
