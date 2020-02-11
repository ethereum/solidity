contract Test {
    mapping(string => mapping(string => uint)) public data;

    function set(string memory _s, string memory _s2, uint _v) public {
        data[_s][_s2] = _v;
    }
}

// ----
// callContractFunction( "set(string,string,uint256): 0x60), roundTo32(0x80 + strings[i].size())), 7 + i), strings[i].size()), strings[i], strings[i+1].size()), strings[i+1]  -> 
// set(string,string,uint256):"96, 160, 7, 13, Hello, World!, 87, Hello,                            World!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1111" -> ""
// callContractFunction( "set(string,string,uint256): 0x60), roundTo32(0x80 + strings[i].size())), 7 + i), strings[i].size()), strings[i], strings[i+1].size()), strings[i+1]  -> 
// set(string,string,uint256):"96, 224, 8, 87, Hello,                            World!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1111, 0, " -> ""
// callContractFunction( "set(string,string,uint256): 0x60), roundTo32(0x80 + strings[i].size())), 7 + i), strings[i].size()), strings[i], strings[i+1].size()), strings[i+1]  -> 
// set(string,string,uint256):"96, 128, 9, 0, , 1, 1" -> ""
// callContractFunction( "set(string,string,uint256): 0x60), roundTo32(0x80 + strings[i].size())), 7 + i), strings[i].size()), strings[i], strings[i+1].size()), strings[i+1]  -> 
// set(string,string,uint256):"96, 160, 10, 1, 1, 8, last one" -> ""
// callContractFunction( "data(string,string): 0x40), roundTo32(0x60 + strings[i].size())), strings[i].size()), strings[i], strings[i+1].size()), strings[i+1]  -> 7 + i
// data(string,string):"64, 128, 13, Hello, World!, 87, Hello,                            World!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1111" -> "7"
// callContractFunction( "data(string,string): 0x40), roundTo32(0x60 + strings[i].size())), strings[i].size()), strings[i], strings[i+1].size()), strings[i+1]  -> 7 + i
// data(string,string):"64, 192, 87, Hello,                            World!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1111, 0, " -> "8"
// callContractFunction( "data(string,string): 0x40), roundTo32(0x60 + strings[i].size())), strings[i].size()), strings[i], strings[i+1].size()), strings[i+1]  -> 7 + i
// data(string,string):"64, 96, 0, , 1, 1" -> "9"
// callContractFunction( "data(string,string): 0x40), roundTo32(0x60 + strings[i].size())), strings[i].size()), strings[i], strings[i+1].size()), strings[i+1]  -> 7 + i
// data(string,string):"64, 128, 1, 1, 8, last one" -> "10"
