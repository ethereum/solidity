contract Test {
    mapping(string => uint) data;

    function f() public returns(uint) {
        data["abc"] = 2;
        return data["abc"];
    }
}

// ----
// f() -> 2
