contract C {
    function set(uint value) external {
        assembly {
            tstore(0, value)
        }
    }

    function get() external view returns (uint value) {
        assembly {
            value := tload(0)
        }
    }

    function terminate(address payable a) external {
        selfdestruct(a);
    }
}

contract D {
    C public c;

    constructor() {
        c = new C();
    }

    function destroy() external {
        c.set(42);
        c.terminate(payable(address(this)));
        assert(c.get() == 42);
    }

    function createAndDestroy() external {
        c = new C();
        c.set(42);
        c.terminate(payable(address(this)));
        assert(c.get() == 42);
    }
}
// ====
// EVMVersion: >=cancun
// ----
// constructor() ->
// gas irOptimized: 127596
// gas irOptimized code: 221000
// gas legacy: 149480
// gas legacy code: 501200
// gas legacyOptimized: 125846
// gas legacyOptimized code: 203400
// destroy() ->
// createAndDestroy() ->
// gas legacy: 67048
// gas legacy code: 92600
// gas legacyOptimized: 65677
// gas legacyOptimized code: 39400
