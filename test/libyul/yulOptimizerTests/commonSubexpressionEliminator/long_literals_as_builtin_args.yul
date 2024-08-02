object "AccessControlDefaultAdminRules4233_14" {
    code {
        let programSize := datasize("AccessControlDefaultAdminRules4233_14")
        let i := mload(64)
        // argument of datasize is a literal of length >32 which shares a common substring
        // of length 32 with programSize, this should not be substituted
        let j := datasize("AccessControlDefaultAdminRules4233_14_deployed")
        codecopy(i, dataoffset("AccessControlDefaultAdminRules4233_14_deployed"), j)
    }
    data "AccessControlDefaultAdminRules4233_14_deployed" "AccessControlDefaultAdminRules4233_14_deployed"

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
//     data "AccessControlDefaultAdminRules4233_14_deployed" hex"416363657373436f6e74726f6c44656661756c7441646d696e52756c6573343233335f31345f6465706c6f796564"
// }
