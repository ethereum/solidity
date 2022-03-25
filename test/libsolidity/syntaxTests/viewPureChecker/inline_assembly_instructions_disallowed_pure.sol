contract C {
    function f() public pure {
        assembly {
            pop(sload(0))
            sstore(0, 1)
            pop(gas())
            pop(address())
            pop(balance(0))
            pop(selfbalance())
            pop(caller())
            pop(callvalue())
            pop(extcodesize(0))
            extcodecopy(0, 1, 2, 3)
            pop(returndatasize())
            returndatacopy(0, 1, 2)
            pop(extcodehash(0))
            pop(create(0, 1, 2))
            pop(create2(0, 1, 2, 3))
            pop(call(0, 1, 2, 3, 4, 5, 6))
            pop(callcode(0, 1, 2, 3, 4, 5, 6))
            pop(delegatecall(0, 1, 2, 3, 4, 5))
            pop(staticcall(0, 1, 2, 3, 4, 5))
            selfdestruct(0)
            log0(0, 1)
            log1(0, 1, 2)
            log2(0, 1, 2, 3)
            log3(0, 1, 2, 3, 4)
            log4(0, 1, 2, 3, 4, 5)
            pop(chainid())
            pop(basefee())
            pop(origin())
            pop(gasprice())
            pop(blockhash(0))
            pop(coinbase())
            pop(timestamp())
            pop(number())
            pop(difficulty())
            pop(gaslimit())

            // These two are disallowed too but the error suppresses other errors.
            //pop(msize())
            //pop(pc())
        }
    }
}
// ====
// EVMVersion: >=london
// ----
// Warning 5740: (742-1153): Unreachable code.
// TypeError 2527: (79-87): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 8961: (101-113): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 2527: (130-135): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (153-162): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (180-190): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (208-221): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (239-247): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (265-276): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (294-308): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (322-345): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (362-378): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (392-415): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (432-446): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 8961: (464-479): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (497-516): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (534-559): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (577-606): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (624-654): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 2527: (672-700): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 8961: (714-729): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (742-752): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (765-778): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (791-807): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (820-839): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (852-874): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 2527: (891-900): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (918-927): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (945-953): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (971-981): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (999-1011): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (1029-1039): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (1057-1068): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (1086-1094): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (1112-1124): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (1142-1152): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
