contract Test {
    mapping(string => uint) public data;

    function set(string memory _s, uint _v) public {
        data[_s] = _v;
    }
}

// ----
// callContractFunction( "set(string,uint256): 0x40), 7 + i), strings[i].size()), strings[i]  -> 
// set(string,uint256):"64, 7, 13, Hello, World!" -> ""
// callContractFunction( "set(string,uint256): 0x40), 7 + i), strings[i].size()), strings[i]  -> 
// set(string,uint256):"64, 8, 87, Hello,                            World!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1111" -> ""
// callContractFunction( "set(string,uint256): 0x40), 7 + i), strings[i].size()), strings[i]  -> 
// set(string,uint256):"64, 9, 0, " -> ""
// callContractFunction( "set(string,uint256): 0x40), 7 + i), strings[i].size()), strings[i]  -> 
// set(string,uint256):"64, 10, 1, 1" -> ""
// callContractFunction( "data(string): 0x20), strings[i].size()), strings[i]  -> 7 + i
// data(string):"32, 13, Hello, World!" -> "7"
// callContractFunction( "data(string): 0x20), strings[i].size()), strings[i]  -> 7 + i
// data(string):"32, 87, Hello,                            World!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1111" -> "8"
// callContractFunction( "data(string): 0x20), strings[i].size()), strings[i]  -> 7 + i
// data(string):"32, 0, " -> "9"
// callContractFunction( "data(string): 0x20), strings[i].size()), strings[i]  -> 7 + i
// data(string):"32, 1, 1" -> "10"
