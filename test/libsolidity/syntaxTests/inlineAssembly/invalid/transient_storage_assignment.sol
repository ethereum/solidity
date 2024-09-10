contract test {
    uint transient x;
    function f() public {
        assembly {
            x := 2
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
// TypeError 1408: (95-96): Only local variables are supported. To access state variables, use the ".slot" and ".offset" suffixes.
