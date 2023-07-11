contract C {
    function f() public pure {
        assembly {
            pop(sload(0))
            sstore(0, 1)
            pop(gas())
            pop(address())
            pop(balance(0))
            pop(caller())
            pop(callvalue())
            pop(extcodesize(0))
            extcodecopy(0, 1, 2, 3)
            pop(create(0, 1, 2))
            pop(call(0, 1, 2, 3, 4, 5, 6))
            pop(callcode(0, 1, 2, 3, 4, 5, 6))
            pop(delegatecall(0, 1, 2, 3, 4, 5))
            selfdestruct(0)
            log0(0, 1)
            log1(0, 1, 2)
            log2(0, 1, 2, 3)
            log3(0, 1, 2, 3, 4)
            log4(0, 1, 2, 3, 4, 5)
            pop(origin())
            pop(gasprice())
            pop(blockhash(0))
            pop(coinbase())
            pop(timestamp())
            pop(number())
            pop(gaslimit())

            // These two are disallowed too but the error suppresses other errors.
            //pop(msize())
            //pop(pc())
        }
    }
}
// ----
// Warning 1699: (498-510): "selfdestruct" has been deprecated. The underlying opcode will eventually undergo breaking changes, and its use is not recommended.
// Warning 5740: (526-853): Unreachable code.
// TypeError 2527: (79-87): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 8961: (101-113): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 2527: (130-135): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (153-162): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (180-190): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (208-216): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (234-245): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (263-277): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (291-314): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 8961: (331-346): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (364-389): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (407-436): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (454-484): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (498-513): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (526-536): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (549-562): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (575-591): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (604-623): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (636-658): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 2527: (675-683): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (701-711): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (729-741): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (759-769): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (787-798): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (816-824): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (842-852): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
