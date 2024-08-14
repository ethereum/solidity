contract D {
    function addOne() external {
        assembly {
            let x := tload(0)
            tstore(0, add(x, 1))
        }
    }
    function get() external returns (uint x) {
        assembly {
            x := tload(0)
        }
    }
}

contract C {
    function set(uint x) external {
        assembly {
            tstore(0, x)
        }
    }

    function get() external view returns (uint x) {
        assembly {
            x := tload(0)
        }
    }

    function testDelegateCall() external returns (bool) {
        this.set(5);
        D d = new D{salt: hex"00"}();
        // Caller contract is the owner of the transient storage
        (bool success, ) = address(d).delegatecall(abi.encodeCall(d.addOne, ()));
        require(success);
        require(this.get() == 6);
        return true;
    }

    function testCall() external returns (bool) {
        this.set(5);
        D d = new D{salt: hex"01"}();
        // Callee/Target contract is the owner of the transient storage
        (bool success, ) = address(d).call(abi.encodeCall(d.addOne, ()));
        require(success);
        require(d.get() == 1);
        return true;
    }

    function tloadAllowedStaticCall() external returns (bool) {
        this.set(5);
        D d = new D{salt: hex"02"}();
        (bool success, bytes memory result) = address(d).staticcall(abi.encodeCall(d.get, ()));
        require(success);
        require(abi.decode(result, (uint)) == 0);
        return true;
    }

    function tstoreNotAllowedStaticCall() external returns (bool) {
        D d = new D{salt: hex"03"}();
        (bool success, ) = address(d).staticcall(abi.encodeCall(d.addOne, ()));
        require(!success);
        return true;
    }
}
// ====
// EVMVersion: >=cancun
// compileToEOF: true
// EVMVersion: >=prague
// ----
// testDelegateCall() -> true
// testCall() -> true
// tloadAllowedStaticCall() -> true
// tstoreNotAllowedStaticCall() -> true
// gas irOptimized: 98419720
// gas irOptimized code: 19000
// gas legacy: 98409086
// gas legacy code: 30000
// gas legacyOptimized: 98420962
// gas legacyOptimized code: 17800
