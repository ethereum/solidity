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
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
