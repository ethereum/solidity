object "AccessControlDefaultAdminRules4233_14" {
    code {
        let programSize := datasize("AccessControlDefaultAdminRules4233_14")
        let i := mload(64)
        // argument of datasize is a literal of length >32 which shares a common substring
        // of length 32 with programSize, this should not be substituted
        let j := datasize("AccessControlDefaultAdminRules4233_14_deployed")
        codecopy(i, dataoffset("AccessControlDefaultAdminRules4233_14_deployed"), j)
    }
    object "AccessControlDefaultAdminRules4233_14_deployed" {
        code {}
    }

}
// ----
// step: commonSubexpressionEliminator
//
// object "AccessControlDefaultAdminRules4233_14" {
//     code {
//         let programSize := datasize("AccessControlDefaultAdminRules4233_14")
//         let i := mload(64)
//         let j := datasize("AccessControlDefaultAdminRules4233_14_deployed")
//         codecopy(i, dataoffset("AccessControlDefaultAdminRules4233_14_deployed"), j)
//     }
//     object "AccessControlDefaultAdminRules4233_14_deployed" {
//         code { }
//     }
// }
