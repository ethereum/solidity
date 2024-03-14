==== Source: A ====
contract C {
    function f() external {
        assembly {
            tstore(0, 0)
            let a := tload(0)
            tstore(0, 1)
            tstore(1, a)
        }
    }
}
==== Source: B ====
import {C as C} from "A";
contract D {
    function g() external {
        assembly {
            tstore(0, 0)
            tstore(0, 1)
        }
    }
}
==== Source: C ====
contract X {
    function h() external {
        assembly {
            tstore(0, 0)
            tstore(0, 0)
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
// Warning 2394: (A:72-78): Transient storage as defined by EIP-1153 can break the composability of smart contracts: Since transient storage is cleared only at the end of the transaction and not at the end of the outermost call frame to the contract within a transaction, your contract may unintentionally misbehave when invoked multiple times in a complex transaction. To avoid this, be sure to clear all transient storage at the end of any call to your contract. The use of transient storage for reentrancy guards that are cleared at the end of the call is safe.
