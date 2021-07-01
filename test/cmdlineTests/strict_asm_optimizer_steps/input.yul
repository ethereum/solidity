object "C_6" {
    code {
        mstore(64, 128)
        if callvalue() { revert(0, 0) }
        codecopy(0, dataoffset("C_6_deployed"), datasize("C_6_deployed"))
        return(0, datasize("C_6_deployed"))
    }
    object "C_6_deployed" {
        code {
            {
                mstore(64, 128)
                if iszero(lt(calldatasize(), 4))
                {
                    let selector := shift_right_224_unsigned(calldataload(0))
                    pop(selector)
                }
                sstore(0, iszero(calldatasize()))
                revert(0, 0)
            }

            function shift_right_224_unsigned(value) -> newValue
            {
                newValue := shr(224, value)
            }
        }
    }
}
