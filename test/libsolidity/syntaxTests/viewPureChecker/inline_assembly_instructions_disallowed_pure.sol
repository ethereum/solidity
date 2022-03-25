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
// Warning 5740: (672-1083): Unreachable code.
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
// TypeError 2527: (362-376): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 8961: (394-409): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (427-446): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (464-489): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (507-536): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (554-584): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 2527: (602-630): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 8961: (644-659): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (672-682): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (695-708): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (721-737): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (750-769): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 8961: (782-804): Function cannot be declared as pure because this expression (potentially) modifies the state.
// TypeError 2527: (821-830): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (848-857): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (875-883): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (901-911): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (929-941): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (959-969): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (987-998): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (1016-1024): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (1042-1054): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (1072-1082): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
