object "object" {
    code {
        mstore(0, eofcreate("sub0", 0, 0, 0, 0))
        mstore(32, eofcreate("sub1", 0, 0, 0, 0))
        return(0, 64)
    }

    object "sub0" {
        code {
            returncontract("sub00", 0, 0)
        }
        object "sub00" {
            code {
                mstore(0, 1)
                revert(0, 32)
            }
        }
    }

    object "sub1" {
        code {
            returncontract("sub10", 0, 0)
        }
        object "sub10" {
            code {
                mstore(0, 0x20)
                revert(0, 32)
            }
        }
    }
    object "sub2" {
        code {
            mstore(0, 3)
            revert(0, 32)
        }
    }
}