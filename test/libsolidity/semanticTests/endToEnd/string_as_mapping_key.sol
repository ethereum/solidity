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
// callContractFunction( "set(string,uint256): 0x40), 7 + i), strings[i].size()), strings[i]  -> 
// set(string,uint256):"64, 7, 13, Hello, World!" -> ""
// callContractFunction( "set(string,uint256): 0x40), 7 + i), strings[i].size()), strings[i]  -> 
// set(string,uint256):"64, 8, 87, Hello,                            World!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1111" -> ""
// callContractFunction( "set(string,uint256): 0x40), 7 + i), strings[i].size()), strings[i]  -> 
// set(string,uint256):"64, 9, 0, " -> ""
// callContractFunction( "set(string,uint256): 0x40), 7 + i), strings[i].size()), strings[i]  -> 
// set(string,uint256):"64, 10, 1, 1" -> ""
// callContractFunction( "get(string): 0x20), strings[i].size()), strings[i]  -> 7 + i
// get(string):"32, 13, Hello, World!" -> "7"
// callContractFunction( "get(string): 0x20), strings[i].size()), strings[i]  -> 7 + i
// get(string):"32, 87, Hello,                            World!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1111" -> "8"
// callContractFunction( "get(string): 0x20), strings[i].size()), strings[i]  -> 7 + i
// get(string):"32, 0, " -> "9"
// callContractFunction( "get(string): 0x20), strings[i].size()), strings[i]  -> 7 + i
// get(string):"32, 1, 1" -> "10"
