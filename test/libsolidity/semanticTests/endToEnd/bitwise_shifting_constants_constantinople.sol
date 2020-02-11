contract C {
    function shl_1() public returns(bool) {
        uint c;
        assembly {
            c: = shl(2, 1)
        }
        assert(c == 4);
        return true;
    }

    function shl_2() public returns(bool) {
        uint c;
        assembly {
            c: = shl(1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
        }
        assert(c == 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe);
        return true;
    }

    function shl_3() public returns(bool) {
        uint c;
        assembly {
            c: = shl(256, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
        }
        assert(c == 0);
        return true;
    }

    function shr_1() public returns(bool) {
        uint c;
        assembly {
            c: = shr(1, 3)
        }
        assert(c == 1);
        return true;
    }

    function shr_2() public returns(bool) {
        uint c;
        assembly {
            c: = shr(1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
        }
        assert(c == 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff);
        return true;
    }

    function shr_3() public returns(bool) {
        uint c;
        assembly {
            c: = shr(256, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
        }
        assert(c == 0);
        return true;
    }
}

// ====
// compileViaYul: also
// ----
shl_1(): ""
shl_2(): ""
shl_3(): ""
shr_1(): ""
shr_2(): ""
shr_3(): ""
