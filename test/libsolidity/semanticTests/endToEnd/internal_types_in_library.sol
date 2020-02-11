library Lib {
    function find(uint16[] storage _haystack, uint16 _needle) public view returns(uint) {
        for (uint i = 0; i < _haystack.length; ++i)
            if (_haystack[i] == _needle)
                return i;
        return uint(-1);
    }
}
contract Test {
    mapping(string => uint16[]) data;

    function f() public returns(uint a, uint b) {
        while (data["abc"].length < 20)
            data["abc"].push();
        data["abc"][4] = 9;
        data["abc"][17] = 3;
        a = Lib.find(data["abc"], 9);
        b = Lib.find(data["abc"], 3);
    }
}

// ----

library Lib {
    function find(uint16[] storage _haystack, uint16 _needle) public view returns(uint) {
        for (uint i = 0; i < _haystack.length; ++i)
            if (_haystack[i] == _needle)
                return i;
        return uint(-1);
    }
}
contract Test {
    mapping(string => uint16[]) data;

    function f() public returns(uint a, uint b) {
        while (data["abc"].length < 20)
            data["abc"].push();
        data["abc"][4] = 9;
        data["abc"][17] = 3;
        a = Lib.find(data["abc"], 9);
        b = Lib.find(data["abc"], 3);
    }
}

// ----
// f() -> 4, 17
// f():"" -> "4, 17"
