contract test {
    function x() public returns (uint) { return 1; }
    function y() public returns (uint) { return 2; }

    function f(bool cond) public returns (uint) {
        function () returns (uint) z = cond ? x : y;
        return z();
    }
}
// ----
// f(bool): true -> 1
// f(bool): false -> 2
