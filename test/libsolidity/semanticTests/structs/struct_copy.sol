contract c {
    struct Nested {
        uint256 x;
        uint256 y;
    }
    struct Struct {
        uint256 a;
        Nested nested;
        uint256 c;
    }
    mapping(uint256 => Struct) data;

    function set(uint256 k) public returns (bool) {
        data[k].a = 1;
        data[k].nested.x = 3;
        data[k].nested.y = 4;
        data[k].c = 2;
        return true;
    }

    function copy(uint256 from, uint256 to) public returns (bool) {
        data[to] = data[from];
        return true;
    }

    function retrieve(uint256 k)
        public
        returns (uint256 a, uint256 x, uint256 y, uint256 c)
    {
        a = data[k].a;
        x = data[k].nested.x;
        y = data[k].nested.y;
        c = data[k].c;
    }
}

// ====
// compileViaYul: also
// ----
// set(uint256): 7 -> true
// gas irOptimized: 110011
// gas legacy: 110616
// gas legacyOptimized: 109995
// retrieve(uint256): 7 -> 1, 3, 4, 2
// copy(uint256,uint256): 7, 8 -> true
// gas irOptimized: 118707
// gas legacy: 119166
// gas legacyOptimized: 118606
// retrieve(uint256): 7 -> 1, 3, 4, 2
// retrieve(uint256): 8 -> 1, 3, 4, 2
// copy(uint256,uint256): 0, 7 -> true
// retrieve(uint256): 7 -> 0, 0, 0, 0
// retrieve(uint256): 8 -> 1, 3, 4, 2
// copy(uint256,uint256): 7, 8 -> true
// retrieve(uint256): 8 -> 0, 0, 0, 0
