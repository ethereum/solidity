object "RunsTest1" {
    code {
        // Deploy the contract
        datacopy(0, dataoffset("Runtime_deployed"), datasize("Runtime_deployed"))
        return(0, datasize("Runtime_deployed"))
    }
    object "Runtime_deployed" {
        code {
            let funcSel := shl(224, 0xabc12345)
            sstore(0, funcSel)
        }
    }
}
