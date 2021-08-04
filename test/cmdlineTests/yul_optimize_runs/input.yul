object "RunsTest1" {
    code {
        // Deploy the contract
        datacopy(0, dataoffset("Runtime"), datasize("Runtime"))
        return(0, datasize("Runtime"))
    }
    object "Runtime" {
        code {
            let funcSel := shl(224, 0xabc12345)
            mstore(0, funcSel)
        }
    }
}
