contract Test {
    mapping(string => uint) public data;

    function set(string memory _s, uint _v) public {
        data[_s] = _v;
    }
}

// ----
// set(string,uint256): 64, 7, 13, "Hello, World!" -> 
// set(string,uint256): 64, 9, 0, "" -> 
// set(string,uint256): 64, 10, 1, "1" -> 
// data(string): 32, 13, "Hello, World!" -> 7
// data(string): 32, 0, "" -> 9
// data(string): 32, 1, "1" -> 10
