object "root" {
    code {
        sstore(0, datasize("root"))
        sstore(1, datasize("0"))
        sstore(2, datasize("1"))
        sstore(3, datasize("2"))
        sstore(4, datasize("3"))
        sstore(5, datasize("4"))
        sstore(6, datasize("5"))
        sstore(7, datasize("6"))
        sstore(8, datasize("7"))
        sstore(9, datasize("8"))
        sstore(10, datasize("9"))
        sstore(11, datasize("a"))
        sstore(12, datasize("b"))
        sstore(13, datasize("c"))
        sstore(14, datasize("d"))
        sstore(15, datasize("e"))
        sstore(16, datasize("f"))
        sstore(17, datasize("10"))
    }

    object "0" {
        code {
            sstore(100, 0)
            sstore(200, datasize("sub0"))
        }
        object "sub0" {
            code {
                sstore(300, 0)
            }
        }
    }

    object "1" {
        code {
            sstore(100, 1)
        }
    }

    object "2" {
        code {
            sstore(101, 2)
        }
    }

    object "3" {
        code {
            sstore(102, 3)
        }
    }

    object "4" {
        code {
            sstore(103, 4)
        }
    }

    object "5" {
        code {
            sstore(104, 5)
        }
    }

    object "6" {
        code {
            sstore(105, 6)
        }
    }

    object "7" {
        code {
            sstore(106, 7)
        }
    }

    object "8" {
        code {
            sstore(107, 8)
        }
    }

    object "9" {
        code {
            sstore(108, 9)
        }
    }

    object "a" {
        code {
            sstore(109, 10)
        }
    }

    object "b" {
        code {
            sstore(110, 11)
        }
    }

    object "c" {
        code {
            sstore(111, 12)
        }
    }

    object "d" {
        code {
            sstore(112, 13)
        }
    }

    object "e" {
        code {
            sstore(113, 14)
        }
    }

    object "f" {
        code {
            sstore(114, 15)
        }
    }

    object "10" {
        code {
            sstore(115, 16)
            sstore(201, datasize("sub10"))
        }
        object "sub10" {
            code {
                sstore(300, 16)
            }
        }
    }
}
// ----
// Assembly:
//     /* "source":45:61   */
//   bytecodeSize
//     /* "source":42:43   */
//   0x00
//     /* "source":35:62   */
//   sstore
//     /* "source":81:94   */
//   dataSize(sub_0)
//     /* "source":78:79   */
//   0x01
//     /* "source":71:95   */
//   sstore
//     /* "source":114:127   */
//   dataSize(sub_1)
//     /* "source":111:112   */
//   0x02
//     /* "source":104:128   */
//   sstore
//     /* "source":147:160   */
//   dataSize(sub_2)
//     /* "source":144:145   */
//   0x03
//     /* "source":137:161   */
//   sstore
//     /* "source":180:193   */
//   dataSize(sub_3)
//     /* "source":177:178   */
//   0x04
//     /* "source":170:194   */
//   sstore
//     /* "source":213:226   */
//   dataSize(sub_4)
//     /* "source":210:211   */
//   0x05
//     /* "source":203:227   */
//   sstore
//     /* "source":246:259   */
//   dataSize(sub_5)
//     /* "source":243:244   */
//   0x06
//     /* "source":236:260   */
//   sstore
//     /* "source":279:292   */
//   dataSize(sub_6)
//     /* "source":276:277   */
//   0x07
//     /* "source":269:293   */
//   sstore
//     /* "source":312:325   */
//   dataSize(sub_7)
//     /* "source":309:310   */
//   0x08
//     /* "source":302:326   */
//   sstore
//     /* "source":345:358   */
//   dataSize(sub_8)
//     /* "source":342:343   */
//   0x09
//     /* "source":335:359   */
//   sstore
//     /* "source":379:392   */
//   dataSize(sub_9)
//     /* "source":375:377   */
//   0x0a
//     /* "source":368:393   */
//   sstore
//     /* "source":413:426   */
//   dataSize(sub_10)
//     /* "source":409:411   */
//   0x0b
//     /* "source":402:427   */
//   sstore
//     /* "source":447:460   */
//   dataSize(sub_11)
//     /* "source":443:445   */
//   0x0c
//     /* "source":436:461   */
//   sstore
//     /* "source":481:494   */
//   dataSize(sub_12)
//     /* "source":477:479   */
//   0x0d
//     /* "source":470:495   */
//   sstore
//     /* "source":515:528   */
//   dataSize(sub_13)
//     /* "source":511:513   */
//   0x0e
//     /* "source":504:529   */
//   sstore
//     /* "source":549:562   */
//   dataSize(sub_14)
//     /* "source":545:547   */
//   0x0f
//     /* "source":538:563   */
//   sstore
//     /* "source":583:596   */
//   dataSize(sub_15)
//     /* "source":579:581   */
//   0x10
//     /* "source":572:597   */
//   sstore
//     /* "source":617:631   */
//   dataSize(sub_16)
//     /* "source":613:615   */
//   0x11
//     /* "source":606:632   */
//   sstore
//     /* "source":25:638   */
//   stop
// stop
//
// sub_0: assembly {
//         /* "source":696:697   */
//       0x00
//         /* "source":691:694   */
//       0x64
//         /* "source":684:698   */
//       sstore
//         /* "source":723:739   */
//       dataSize(sub_0)
//         /* "source":718:721   */
//       0xc8
//         /* "source":711:740   */
//       sstore
//         /* "source":670:750   */
//       stop
//     stop
//
//     sub_0: assembly {
//             /* "source":822:823   */
//           0x00
//             /* "source":817:820   */
//           0x012c
//             /* "source":810:824   */
//           sstore
//             /* "source":792:838   */
//           stop
//     }
// }
//
// sub_1: assembly {
//         /* "source":912:913   */
//       0x01
//         /* "source":907:910   */
//       0x64
//         /* "source":900:914   */
//       sstore
//         /* "source":886:924   */
//       stop
// }
//
// sub_2: assembly {
//         /* "source":988:989   */
//       0x02
//         /* "source":983:986   */
//       0x65
//         /* "source":976:990   */
//       sstore
//         /* "source":962:1000   */
//       stop
// }
//
// sub_3: assembly {
//         /* "source":1064:1065   */
//       0x03
//         /* "source":1059:1062   */
//       0x66
//         /* "source":1052:1066   */
//       sstore
//         /* "source":1038:1076   */
//       stop
// }
//
// sub_4: assembly {
//         /* "source":1140:1141   */
//       0x04
//         /* "source":1135:1138   */
//       0x67
//         /* "source":1128:1142   */
//       sstore
//         /* "source":1114:1152   */
//       stop
// }
//
// sub_5: assembly {
//         /* "source":1216:1217   */
//       0x05
//         /* "source":1211:1214   */
//       0x68
//         /* "source":1204:1218   */
//       sstore
//         /* "source":1190:1228   */
//       stop
// }
//
// sub_6: assembly {
//         /* "source":1292:1293   */
//       0x06
//         /* "source":1287:1290   */
//       0x69
//         /* "source":1280:1294   */
//       sstore
//         /* "source":1266:1304   */
//       stop
// }
//
// sub_7: assembly {
//         /* "source":1368:1369   */
//       0x07
//         /* "source":1363:1366   */
//       0x6a
//         /* "source":1356:1370   */
//       sstore
//         /* "source":1342:1380   */
//       stop
// }
//
// sub_8: assembly {
//         /* "source":1444:1445   */
//       0x08
//         /* "source":1439:1442   */
//       0x6b
//         /* "source":1432:1446   */
//       sstore
//         /* "source":1418:1456   */
//       stop
// }
//
// sub_9: assembly {
//         /* "source":1520:1521   */
//       0x09
//         /* "source":1515:1518   */
//       0x6c
//         /* "source":1508:1522   */
//       sstore
//         /* "source":1494:1532   */
//       stop
// }
//
// sub_10: assembly {
//         /* "source":1596:1598   */
//       0x0a
//         /* "source":1591:1594   */
//       0x6d
//         /* "source":1584:1599   */
//       sstore
//         /* "source":1570:1609   */
//       stop
// }
//
// sub_11: assembly {
//         /* "source":1673:1675   */
//       0x0b
//         /* "source":1668:1671   */
//       0x6e
//         /* "source":1661:1676   */
//       sstore
//         /* "source":1647:1686   */
//       stop
// }
//
// sub_12: assembly {
//         /* "source":1750:1752   */
//       0x0c
//         /* "source":1745:1748   */
//       0x6f
//         /* "source":1738:1753   */
//       sstore
//         /* "source":1724:1763   */
//       stop
// }
//
// sub_13: assembly {
//         /* "source":1827:1829   */
//       0x0d
//         /* "source":1822:1825   */
//       0x70
//         /* "source":1815:1830   */
//       sstore
//         /* "source":1801:1840   */
//       stop
// }
//
// sub_14: assembly {
//         /* "source":1904:1906   */
//       0x0e
//         /* "source":1899:1902   */
//       0x71
//         /* "source":1892:1907   */
//       sstore
//         /* "source":1878:1917   */
//       stop
// }
//
// sub_15: assembly {
//         /* "source":1981:1983   */
//       0x0f
//         /* "source":1976:1979   */
//       0x72
//         /* "source":1969:1984   */
//       sstore
//         /* "source":1955:1994   */
//       stop
// }
//
// sub_16: assembly {
//         /* "source":2059:2061   */
//       0x10
//         /* "source":2054:2057   */
//       0x73
//         /* "source":2047:2062   */
//       sstore
//         /* "source":2087:2104   */
//       dataSize(sub_0)
//         /* "source":2082:2085   */
//       0xc9
//         /* "source":2075:2105   */
//       sstore
//         /* "source":2033:2115   */
//       stop
//     stop
//
//     sub_0: assembly {
//             /* "source":2188:2190   */
//           0x10
//             /* "source":2183:2186   */
//           0x012c
//             /* "source":2176:2191   */
//           sstore
//             /* "source":2158:2205   */
//           stop
//     }
// }
// Bytecode: 61005c5f55600b600155600660025560066003556006600455600660055560066006556006600755600660085560066009556006600a556006600b556006600c556006600d556006600e556006600f556006601055600c60115500fe
// Opcodes: PUSH2 0x5C PUSH0 SSTORE PUSH1 0xB PUSH1 0x1 SSTORE PUSH1 0x6 PUSH1 0x2 SSTORE PUSH1 0x6 PUSH1 0x3 SSTORE PUSH1 0x6 PUSH1 0x4 SSTORE PUSH1 0x6 PUSH1 0x5 SSTORE PUSH1 0x6 PUSH1 0x6 SSTORE PUSH1 0x6 PUSH1 0x7 SSTORE PUSH1 0x6 PUSH1 0x8 SSTORE PUSH1 0x6 PUSH1 0x9 SSTORE PUSH1 0x6 PUSH1 0xA SSTORE PUSH1 0x6 PUSH1 0xB SSTORE PUSH1 0x6 PUSH1 0xC SSTORE PUSH1 0x6 PUSH1 0xD SSTORE PUSH1 0x6 PUSH1 0xE SSTORE PUSH1 0x6 PUSH1 0xF SSTORE PUSH1 0x6 PUSH1 0x10 SSTORE PUSH1 0xC PUSH1 0x11 SSTORE STOP INVALID
// SourceMappings: 45:16:0:-:0;42:1;35:27;81:13;78:1;71:24;114:13;111:1;104:24;147:13;144:1;137:24;180:13;177:1;170:24;213:13;210:1;203:24;246:13;243:1;236:24;279:13;276:1;269:24;312:13;309:1;302:24;345:13;342:1;335:24;379:13;375:2;368:25;413:13;409:2;402:25;447:13;443:2;436:25;481:13;477:2;470:25;515:13;511:2;504:25;549:13;545:2;538:25;583:13;579:2;572:25;617:14;613:2;606:26;25:613
