object "Myobj"
{
    code {
        function ops(id) -> r {
            switch id
            case 0 { r := 1 }
            // --------------- blockchain stuff ---------------
            case 1 { let _x := keccak256(0, 0) }
            case 2 { let _x := address() }
            case 3 { let _x := balance(0) }
            case 4 { let _x := selfbalance() }
            case 5 { let _x := origin() }
            case 6 { let _x := caller() }
            case 7 { let _x := calldataload(0) }
            case 8 { let _x := calldatasize() }
            case 9 { calldatacopy(0, 0, 0) }
            case 10 { let _x := codesize() }
            case 11 { codecopy(0, 0, 0) }
            case 12 { let _x := gasprice() }
            case 13 { let _x := chainid() }
            case 14 { let _x := blobhash(0) }
            case 15 { let _x := blobbasefee() }
            case 16 { let _x := extcodesize(0) }
            case 17 { let _x := extcodehash(0) }
            case 18 { extcodecopy(0, 0, 0, 0) }
            case 19 { let _x := returndatasize() }
            case 20 { returndatacopy(0, 0, 0) }
            case 21 { mcopy(0, 0, 0) }
            case 22 { let _x := blockhash(0) }
            case 23 { let _x := coinbase() }
            case 24 { let _x := timestamp() }
            case 25 { let _x := number() }
            case 26 { let _x := prevrandao() }
            case 27 { let _x := gaslimit() }
            // --------------- memory / storage / logs ---------------
            case 28 { let _x := mload(0) }
            case 29 { mstore(0, 0) }
            case 30 { mstore8(0, 0) }
            case 31 { let _x := sload(0) }
            case 32 { sstore(0, 0) }
            // can not use PC here
            // case 33 { let _x := pc() }
            case 33 { revert(0, 0) }
            case 34 { let _x := msize() }
            case 35 { let _x := gas() }
            case 36 { log0(0, 0) }
            case 37 { log1(0, 0, 0) }
            case 38 { log2(0, 0, 0, 0) }
            case 39 { log3(0, 0, 0, 0, 0) }
            case 40 { log4(0, 0, 0, 0, 0, 0) }
            case 41 { let _x := tload(0) }
            case 42 { tstore(0, 0) }
            // --------------- calls ---------------
            case 43 { let _x := create(0, 0, 0) }
            case 44 { let _x := create2(0, 0, 0, 0) }
            case 45 { let _x := call(0, 0, 0, 0, 0, 0, 0) }
            case 46 { let _x := callcode(0, 0, 0, 0, 0, 0, 0) }
            case 47 { let _x := delegatecall(0, 0, 0, 0, 0, 0) }
            case 48 { let _x := staticcall(0, 0, 0, 0, 0, 0) }
            case 49 { return(0, 0) }
            case 50 { revert(0, 0) }
            case 51 { invalid() }
            case 52 { selfdestruct(0) }
            // --------------- meta ----------------
            case 53 { let _x := datasize("Mydata") }
            case 54 { let _x := dataoffset("Mydata") }
            case 55 { datacopy("Mydata", 0, 0) }
            case 56 { let _x := loadimmutable("x") }
            case 57 { setimmutable(0, "x", 0) }
            case 58 { let a := linkersymbol("x") }
            case 59 { let _x := memoryguard(0) }
            case 60 { verbatim_1i_0o(hex"deadbeef", 0) }
            case 61 { let _x := verbatim_1i_1o(hex"deadbeef", 0) }
            // --------------- stop ---------------
            case 62 { stop() }
            // should check if value is greater than 62
            default { revert(0, 0) }
        }
        function test_0() -> r { r := ops(0) }
        function test_1() -> r { r := ops(1) }
        function test_2() -> r { r := ops(2) }
        function test_3() -> r { r := ops(3) }
        function test_4() -> r { r := ops(4) }
        function test_5() -> r { r := ops(5) }
        function test_6() -> r { r := ops(6) }
        function test_7() -> r { r := ops(7) }
        function test_8() -> r { r := ops(8) }
        function test_9() -> r { r := ops(9) }
        function test_10() -> r { r := ops(10) }
        function test_11() -> r { r := ops(11) }
        function test_12() -> r { r := ops(12) }
        function test_13() -> r { r := ops(13) }
        function test_14() -> r { r := ops(14) }
        function test_15() -> r { r := ops(15) }
        function test_16() -> r { r := ops(16) }
        function test_17() -> r { r := ops(17) }
        function test_18() -> r { r := ops(18) }
        function test_19() -> r { r := ops(19) }
        function test_20() -> r { r := ops(20) }
        function test_21() -> r { r := ops(21) }
        function test_22() -> r { r := ops(22) }
        function test_23() -> r { r := ops(23) }
        function test_24() -> r { r := ops(24) }
        function test_25() -> r { r := ops(25) }
        function test_26() -> r { r := ops(26) }
        function test_27() -> r { r := ops(27) }
        function test_28() -> r { r := ops(28) }
        function test_29() -> r { r := ops(29) }
        function test_30() -> r { r := ops(30) }
        function test_31() -> r { r := ops(31) }
        function test_32() -> r { r := ops(32) }
        function test_33() -> r { r := ops(33) }
        function test_34() -> r { r := ops(34) }
        function test_35() -> r { r := ops(35) }
        function test_36() -> r { r := ops(36) }
        function test_37() -> r { r := ops(37) }
        function test_38() -> r { r := ops(38) }
        function test_39() -> r { r := ops(39) }
        function test_40() -> r { r := ops(40) }
        function test_41() -> r { r := ops(41) }
        function test_42() -> r { r := ops(42) }
        function test_43() -> r { r := ops(43) }
        function test_44() -> r { r := ops(44) }
        function test_45() -> r { r := ops(45) }
        function test_46() -> r { r := ops(46) }
        function test_47() -> r { r := ops(47) }
        function test_48() -> r { r := ops(48) }
        function test_49() -> r { r := ops(49) }
        function test_50() -> r { r := ops(50) }
        function test_51() -> r { r := ops(51) }
        function test_52() -> r { r := ops(52) }
        function test_53() -> r { r := ops(53) }
        function test_54() -> r { r := ops(54) }
        function test_55() -> r { r := ops(55) }
        function test_56() -> r { r := ops(56) }
        function test_57() -> r { r := ops(57) }
        function test_58() -> r { r := ops(58) }
        function test_59() -> r { r := ops(59) }
        function test_60() -> r { r := ops(60) }
        function test_61() -> r { r := ops(61) }
        function test_62() -> r { r := ops(62) }
    }
    data "Mydata" hex"1234"
}
// ====
// EVMVersion: >=cancun
// ----
// step: constantFunctionEvaluator
//
// object "Myobj" {
//     code {
//         function ops(id) -> r
//         {
//             switch id
//             case 0 { r := 1 }
//             case 1 { let _x := keccak256(0, 0) }
//             case 2 { let _x_1 := address() }
//             case 3 { let _x_2 := balance(0) }
//             case 4 { let _x_3 := selfbalance() }
//             case 5 { let _x_4 := origin() }
//             case 6 { let _x_5 := caller() }
//             case 7 { let _x_6 := calldataload(0) }
//             case 8 { let _x_7 := calldatasize() }
//             case 9 { calldatacopy(0, 0, 0) }
//             case 10 { let _x_8 := codesize() }
//             case 11 { codecopy(0, 0, 0) }
//             case 12 { let _x_9 := gasprice() }
//             case 13 { let _x_10 := chainid() }
//             case 14 { let _x_11 := blobhash(0) }
//             case 15 { let _x_12 := blobbasefee() }
//             case 16 { let _x_13 := extcodesize(0) }
//             case 17 { let _x_14 := extcodehash(0) }
//             case 18 { extcodecopy(0, 0, 0, 0) }
//             case 19 { let _x_15 := returndatasize() }
//             case 20 { returndatacopy(0, 0, 0) }
//             case 21 { mcopy(0, 0, 0) }
//             case 22 { let _x_16 := blockhash(0) }
//             case 23 { let _x_17 := coinbase() }
//             case 24 { let _x_18 := timestamp() }
//             case 25 { let _x_19 := number() }
//             case 26 { let _x_20 := prevrandao() }
//             case 27 { let _x_21 := gaslimit() }
//             case 28 { let _x_22 := mload(0) }
//             case 29 { mstore(0, 0) }
//             case 30 { mstore8(0, 0) }
//             case 31 { let _x_23 := sload(0) }
//             case 32 { sstore(0, 0) }
//             case 33 { revert(0, 0) }
//             case 34 { let _x_24 := msize() }
//             case 35 { let _x_25 := gas() }
//             case 36 { log0(0, 0) }
//             case 37 { log1(0, 0, 0) }
//             case 38 { log2(0, 0, 0, 0) }
//             case 39 { log3(0, 0, 0, 0, 0) }
//             case 40 { log4(0, 0, 0, 0, 0, 0) }
//             case 41 { let _x_26 := tload(0) }
//             case 42 { tstore(0, 0) }
//             case 43 { let _x_27 := create(0, 0, 0) }
//             case 44 {
//                 let _x_28 := create2(0, 0, 0, 0)
//             }
//             case 45 {
//                 let _x_29 := call(0, 0, 0, 0, 0, 0, 0)
//             }
//             case 46 {
//                 let _x_30 := callcode(0, 0, 0, 0, 0, 0, 0)
//             }
//             case 47 {
//                 let _x_31 := delegatecall(0, 0, 0, 0, 0, 0)
//             }
//             case 48 {
//                 let _x_32 := staticcall(0, 0, 0, 0, 0, 0)
//             }
//             case 49 { return(0, 0) }
//             case 50 { revert(0, 0) }
//             case 51 { invalid() }
//             case 52 { selfdestruct(0) }
//             case 53 {
//                 let _x_33 := datasize("Mydata")
//             }
//             case 54 {
//                 let _x_34 := dataoffset("Mydata")
//             }
//             case 55 { datacopy("Mydata", 0, 0) }
//             case 56 {
//                 let _x_35 := loadimmutable("x")
//             }
//             case 57 { setimmutable(0, "x", 0) }
//             case 58 { let a := linkersymbol("x") }
//             case 59 { let _x_36 := memoryguard(0) }
//             case 60 {
//                 verbatim_1i_0o("\xde\xad\xbe\xef", 0)
//             }
//             case 61 {
//                 let _x_37 := verbatim_1i_1o("\xde\xad\xbe\xef", 0)
//             }
//             case 62 { stop() }
//             default { revert(0, 0) }
//         }
//         function test_0() -> r_38
//         { r_38 := 1 }
//         function test_1() -> r_39
//         { r_39 := ops(1) }
//         function test_2() -> r_40
//         { r_40 := ops(2) }
//         function test_3() -> r_41
//         { r_41 := ops(3) }
//         function test_4() -> r_42
//         { r_42 := ops(4) }
//         function test_5() -> r_43
//         { r_43 := ops(5) }
//         function test_6() -> r_44
//         { r_44 := ops(6) }
//         function test_7() -> r_45
//         { r_45 := ops(7) }
//         function test_8() -> r_46
//         { r_46 := ops(8) }
//         function test_9() -> r_47
//         { r_47 := ops(9) }
//         function test_10() -> r_48
//         { r_48 := ops(10) }
//         function test_11() -> r_49
//         { r_49 := ops(11) }
//         function test_12() -> r_50
//         { r_50 := ops(12) }
//         function test_13() -> r_51
//         { r_51 := ops(13) }
//         function test_14() -> r_52
//         { r_52 := ops(14) }
//         function test_15() -> r_53
//         { r_53 := ops(15) }
//         function test_16() -> r_54
//         { r_54 := ops(16) }
//         function test_17() -> r_55
//         { r_55 := ops(17) }
//         function test_18() -> r_56
//         { r_56 := ops(18) }
//         function test_19() -> r_57
//         { r_57 := ops(19) }
//         function test_20() -> r_58
//         { r_58 := ops(20) }
//         function test_21() -> r_59
//         { r_59 := ops(21) }
//         function test_22() -> r_60
//         { r_60 := ops(22) }
//         function test_23() -> r_61
//         { r_61 := ops(23) }
//         function test_24() -> r_62
//         { r_62 := ops(24) }
//         function test_25() -> r_63
//         { r_63 := ops(25) }
//         function test_26() -> r_64
//         { r_64 := ops(26) }
//         function test_27() -> r_65
//         { r_65 := ops(27) }
//         function test_28() -> r_66
//         { r_66 := ops(28) }
//         function test_29() -> r_67
//         { r_67 := ops(29) }
//         function test_30() -> r_68
//         { r_68 := ops(30) }
//         function test_31() -> r_69
//         { r_69 := ops(31) }
//         function test_32() -> r_70
//         { r_70 := ops(32) }
//         function test_33() -> r_71
//         { r_71 := ops(33) }
//         function test_34() -> r_72
//         { r_72 := ops(34) }
//         function test_35() -> r_73
//         { r_73 := ops(35) }
//         function test_36() -> r_74
//         { r_74 := ops(36) }
//         function test_37() -> r_75
//         { r_75 := ops(37) }
//         function test_38() -> r_76
//         { r_76 := ops(38) }
//         function test_39() -> r_77
//         { r_77 := ops(39) }
//         function test_40() -> r_78
//         { r_78 := ops(40) }
//         function test_41() -> r_79
//         { r_79 := ops(41) }
//         function test_42() -> r_80
//         { r_80 := ops(42) }
//         function test_43() -> r_81
//         { r_81 := ops(43) }
//         function test_44() -> r_82
//         { r_82 := ops(44) }
//         function test_45() -> r_83
//         { r_83 := ops(45) }
//         function test_46() -> r_84
//         { r_84 := ops(46) }
//         function test_47() -> r_85
//         { r_85 := ops(47) }
//         function test_48() -> r_86
//         { r_86 := ops(48) }
//         function test_49() -> r_87
//         { r_87 := ops(49) }
//         function test_50() -> r_88
//         { r_88 := ops(50) }
//         function test_51() -> r_89
//         { r_89 := ops(51) }
//         function test_52() -> r_90
//         { r_90 := ops(52) }
//         function test_53() -> r_91
//         { r_91 := ops(53) }
//         function test_54() -> r_92
//         { r_92 := ops(54) }
//         function test_55() -> r_93
//         { r_93 := ops(55) }
//         function test_56() -> r_94
//         { r_94 := ops(56) }
//         function test_57() -> r_95
//         { r_95 := ops(57) }
//         function test_58() -> r_96
//         { r_96 := ops(58) }
//         function test_59() -> r_97
//         { r_97 := ops(59) }
//         function test_60() -> r_98
//         { r_98 := ops(60) }
//         function test_61() -> r_99
//         { r_99 := ops(61) }
//         function test_62() -> r_100
//         { r_100 := ops(62) }
//     }
//     data "Mydata" hex"1234"
// }
