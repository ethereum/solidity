contract C {
    function f() external returns(uint a) {
        assembly {
            tstore(0, 13)
            a := tload(0)
        }
    }
}
