contract Test {
    mapping(string => uint256) data;

    function f() public returns (uint256) {
        data["abc"] = 2;
        return data["abc"];
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 2
