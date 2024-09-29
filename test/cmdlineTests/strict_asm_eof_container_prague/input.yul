object "object" {
    code {
        revert(0, 0)
    }

    object "sub0" {
        code {
            mstore(0, 0)
            revert(0, 32)
        }
    }
    object "sub1" {
        code {
            mstore(0, 1)
            revert(0, 32)
        }
    }
    object "sub2" {
        code {
            mstore(0, 2)
            revert(0, 32)
        }
        object "sub20" {
            code {
                mstore(0, 0x20)
                revert(0, 32)
            }
        }
    }
    object "sub3" {
        code {
            mstore(0, 3)
            revert(0, 32)
        }
    }
}