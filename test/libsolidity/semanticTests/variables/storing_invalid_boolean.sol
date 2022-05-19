contract C {
    event Ev(bool);
    bool public perm;
    function set() public returns(uint) {
        bool tmp;
        assembly {
            tmp := 5
        }
        perm = tmp;
        return 1;
    }
    function ret() public returns(bool) {
        bool tmp;
        assembly {
            tmp := 5
        }
        return tmp;
    }
    function ev() public returns(uint) {
        bool tmp;
        assembly {
            tmp := 5
        }
        emit Ev(tmp);
        return 1;
    }
}
// ----
// set() -> 1
// perm() -> true
// ret() -> true
// ev() -> 1
// ~ emit Ev(bool): true
