contract C {
    function f() public pure returns (bool){
        bool flag;
        ((flag = true) ? (1, 2, 3) : (3, 2, 1));
        return flag;
    }
}
// ----
// f() -> true
