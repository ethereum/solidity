function asUint(uint x) pure suffix returns (int) {}
function asSint(int x) pure suffix returns (int) {}
function asUdec(uint m, uint e) pure suffix returns (int) {}
function asSdec(int m, uint e) pure suffix returns (int) {}

contract C {
    function f() public pure {
        1 asUint;
        1 asSint;
        1 asUdec;
        1 asSdec;

        -1 asUint;
        -1 asSint;
        -1 asUdec;
        -1 asSdec;

        1.1 asUdec;
        1.1 asSdec;

        -1.1 asUdec;
        -1.1 asSdec;
    }
}
