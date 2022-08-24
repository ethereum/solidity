pragma abicoder v2;

contract C {
    uint256[][] a1;
    uint256[][2] a2;
    uint256[2][] a3;
    uint256[2][2] a4;

    constructor() {
        a1 = new uint256[][](2);
        a1[0] = [1, 2];
        a1[1] = [3, 4, 5];

        a2[0] = [6, 7, 8];
        a2[1] = [9];

        a3.push([1, 2]);
        a3.push([3, 4]);
        a3.push([5, 6]);

        a4 = [[10, 11], [12, 13]];
    }

    function test1() external returns (uint256[][] memory) {
        return a1;
    }

    function test2() external returns (uint256[][2] memory) {
        return a2;
    }

    function test3() external returns (uint256[2][] memory) {
        return a3;
    }

    function test4() external returns (uint256[2][2] memory) {
        return a4;
    }
}

// ----
// test1() -> 0x20, 2, 0x40, 0xa0, 2, 1, 2, 3, 3, 4, 5
// test2() -> 0x20, 0x40, 0xc0, 3, 6, 7, 8, 1, 9
// test3() -> 0x20, 3, 1, 2, 3, 4, 5, 6
// test4() -> 10, 11, 12, 13
