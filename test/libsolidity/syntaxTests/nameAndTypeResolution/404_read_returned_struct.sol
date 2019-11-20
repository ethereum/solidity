pragma experimental ABIEncoderV2;
contract A {
    struct T {
        int x;
        int y;
    }
    function g() public returns (T memory) {
        return this.g();
    }
}
// ----
