function asUint(uint x) pure returns (int) {}
function asSint(int x) pure returns (int) {}
function asUdec(uint m, uint e) pure returns (int) {}
function asSdec(int m, uint e) pure returns (int) {}

contract C {
    function f() public pure {
        1 asUint;
        1 asSint;
        //1 asUdec;  // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
        //1 asSdec;  // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.

        -1 asUint;
        -1 asSint;
        //-1 asUdec; // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.
        //-1 asSdec; // TODO: Needs codegen changes to work. Currently throws in StackHeightChecker.

        1.1 asUdec;
        1.1 asSdec;

        -1.1 asUdec;
        -1.1 asSdec;
    }
}
