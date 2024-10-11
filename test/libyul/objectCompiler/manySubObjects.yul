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
// ====
// EVMVersion: >=shanghai
// ----
// Assembly:
//     /* "source":59:75   */
//   bytecodeSize
//     /* "source":56:57   */
//   0x00
//     /* "source":49:76   */
//   sstore
//     /* "source":99:112   */
//   dataSize(sub_0)
//     /* "source":96:97   */
//   0x01
//     /* "source":89:113   */
//   sstore
//     /* "source":136:149   */
//   dataSize(sub_1)
//     /* "source":133:134   */
//   0x02
//     /* "source":126:150   */
//   sstore
//     /* "source":173:186   */
//   dataSize(sub_2)
//     /* "source":170:171   */
//   0x03
//     /* "source":163:187   */
//   sstore
//     /* "source":210:223   */
//   dataSize(sub_3)
//     /* "source":207:208   */
//   0x04
//     /* "source":200:224   */
//   sstore
//     /* "source":247:260   */
//   dataSize(sub_4)
//     /* "source":244:245   */
//   0x05
//     /* "source":237:261   */
//   sstore
//     /* "source":284:297   */
//   dataSize(sub_5)
//     /* "source":281:282   */
//   0x06
//     /* "source":274:298   */
//   sstore
//     /* "source":321:334   */
//   dataSize(sub_6)
//     /* "source":318:319   */
//   0x07
//     /* "source":311:335   */
//   sstore
//     /* "source":358:371   */
//   dataSize(sub_7)
//     /* "source":355:356   */
//   0x08
//     /* "source":348:372   */
//   sstore
//     /* "source":395:408   */
//   dataSize(sub_8)
//     /* "source":392:393   */
//   0x09
//     /* "source":385:409   */
//   sstore
//     /* "source":433:446   */
//   dataSize(sub_9)
//     /* "source":429:431   */
//   0x0a
//     /* "source":422:447   */
//   sstore
//     /* "source":471:484   */
//   dataSize(sub_10)
//     /* "source":467:469   */
//   0x0b
//     /* "source":460:485   */
//   sstore
//     /* "source":509:522   */
//   dataSize(sub_11)
//     /* "source":505:507   */
//   0x0c
//     /* "source":498:523   */
//   sstore
//     /* "source":547:560   */
//   dataSize(sub_12)
//     /* "source":543:545   */
//   0x0d
//     /* "source":536:561   */
//   sstore
//     /* "source":585:598   */
//   dataSize(sub_13)
//     /* "source":581:583   */
//   0x0e
//     /* "source":574:599   */
//   sstore
//     /* "source":623:636   */
//   dataSize(sub_14)
//     /* "source":619:621   */
//   0x0f
//     /* "source":612:637   */
//   sstore
//     /* "source":661:674   */
//   dataSize(sub_15)
//     /* "source":657:659   */
//   0x10
//     /* "source":650:675   */
//   sstore
//     /* "source":699:713   */
//   dataSize(sub_16)
//     /* "source":695:697   */
//   0x11
//     /* "source":688:714   */
//   sstore
//     /* "source":25:730   */
//   stop
// stop
//
// sub_0: assembly {
//         /* "source":805:806   */
//       0x00
//         /* "source":800:803   */
//       0x64
//         /* "source":793:807   */
//       sstore
//         /* "source":836:852   */
//       dataSize(sub_0)
//         /* "source":831:834   */
//       0xc8
//         /* "source":824:853   */
//       sstore
//         /* "source":761:877   */
//       stop
//     stop
//
//     sub_0: assembly {
//             /* "source":935:936   */
//           0x00
//             /* "source":930:933   */
//           0x012c
//             /* "source":923:937   */
//           sstore
//             /* "source":919:941   */
//           stop
//     }
// }
//
// sub_1: assembly {
//         /* "source":1004:1005   */
//       0x01
//         /* "source":999:1002   */
//       0x64
//         /* "source":992:1006   */
//       sstore
//         /* "source":988:1010   */
//       stop
// }
//
// sub_2: assembly {
//         /* "source":1063:1064   */
//       0x02
//         /* "source":1058:1061   */
//       0x65
//         /* "source":1051:1065   */
//       sstore
//         /* "source":1047:1069   */
//       stop
// }
//
// sub_3: assembly {
//         /* "source":1122:1123   */
//       0x03
//         /* "source":1117:1120   */
//       0x66
//         /* "source":1110:1124   */
//       sstore
//         /* "source":1106:1128   */
//       stop
// }
//
// sub_4: assembly {
//         /* "source":1181:1182   */
//       0x04
//         /* "source":1176:1179   */
//       0x67
//         /* "source":1169:1183   */
//       sstore
//         /* "source":1165:1187   */
//       stop
// }
//
// sub_5: assembly {
//         /* "source":1240:1241   */
//       0x05
//         /* "source":1235:1238   */
//       0x68
//         /* "source":1228:1242   */
//       sstore
//         /* "source":1224:1246   */
//       stop
// }
//
// sub_6: assembly {
//         /* "source":1299:1300   */
//       0x06
//         /* "source":1294:1297   */
//       0x69
//         /* "source":1287:1301   */
//       sstore
//         /* "source":1283:1305   */
//       stop
// }
//
// sub_7: assembly {
//         /* "source":1358:1359   */
//       0x07
//         /* "source":1353:1356   */
//       0x6a
//         /* "source":1346:1360   */
//       sstore
//         /* "source":1342:1364   */
//       stop
// }
//
// sub_8: assembly {
//         /* "source":1417:1418   */
//       0x08
//         /* "source":1412:1415   */
//       0x6b
//         /* "source":1405:1419   */
//       sstore
//         /* "source":1401:1423   */
//       stop
// }
//
// sub_9: assembly {
//         /* "source":1476:1477   */
//       0x09
//         /* "source":1471:1474   */
//       0x6c
//         /* "source":1464:1478   */
//       sstore
//         /* "source":1460:1482   */
//       stop
// }
//
// sub_10: assembly {
//         /* "source":1535:1537   */
//       0x0a
//         /* "source":1530:1533   */
//       0x6d
//         /* "source":1523:1538   */
//       sstore
//         /* "source":1519:1542   */
//       stop
// }
//
// sub_11: assembly {
//         /* "source":1595:1597   */
//       0x0b
//         /* "source":1590:1593   */
//       0x6e
//         /* "source":1583:1598   */
//       sstore
//         /* "source":1579:1602   */
//       stop
// }
//
// sub_12: assembly {
//         /* "source":1655:1657   */
//       0x0c
//         /* "source":1650:1653   */
//       0x6f
//         /* "source":1643:1658   */
//       sstore
//         /* "source":1639:1662   */
//       stop
// }
//
// sub_13: assembly {
//         /* "source":1715:1717   */
//       0x0d
//         /* "source":1710:1713   */
//       0x70
//         /* "source":1703:1718   */
//       sstore
//         /* "source":1699:1722   */
//       stop
// }
//
// sub_14: assembly {
//         /* "source":1775:1777   */
//       0x0e
//         /* "source":1770:1773   */
//       0x71
//         /* "source":1763:1778   */
//       sstore
//         /* "source":1759:1782   */
//       stop
// }
//
// sub_15: assembly {
//         /* "source":1835:1837   */
//       0x0f
//         /* "source":1830:1833   */
//       0x72
//         /* "source":1823:1838   */
//       sstore
//         /* "source":1819:1842   */
//       stop
// }
//
// sub_16: assembly {
//         /* "source":1924:1926   */
//       0x10
//         /* "source":1919:1922   */
//       0x73
//         /* "source":1912:1927   */
//       sstore
//         /* "source":1956:1973   */
//       dataSize(sub_0)
//         /* "source":1951:1954   */
//       0xc9
//         /* "source":1944:1974   */
//       sstore
//         /* "source":1880:1998   */
//       stop
//     stop
//
//     sub_0: assembly {
//             /* "source":2057:2059   */
//           0x10
//             /* "source":2052:2055   */
//           0x012c
//             /* "source":2045:2060   */
//           sstore
//             /* "source":2041:2064   */
//           stop
//     }
// }
// Bytecode: 61005c5f55600b600155600660025560066003556006600455600660055560066006556006600755600660085560066009556006600a556006600b556006600c556006600d556006600e556006600f556006601055600c60115500fe
// Opcodes: PUSH2 0x5C PUSH0 SSTORE PUSH1 0xB PUSH1 0x1 SSTORE PUSH1 0x6 PUSH1 0x2 SSTORE PUSH1 0x6 PUSH1 0x3 SSTORE PUSH1 0x6 PUSH1 0x4 SSTORE PUSH1 0x6 PUSH1 0x5 SSTORE PUSH1 0x6 PUSH1 0x6 SSTORE PUSH1 0x6 PUSH1 0x7 SSTORE PUSH1 0x6 PUSH1 0x8 SSTORE PUSH1 0x6 PUSH1 0x9 SSTORE PUSH1 0x6 PUSH1 0xA SSTORE PUSH1 0x6 PUSH1 0xB SSTORE PUSH1 0x6 PUSH1 0xC SSTORE PUSH1 0x6 PUSH1 0xD SSTORE PUSH1 0x6 PUSH1 0xE SSTORE PUSH1 0x6 PUSH1 0xF SSTORE PUSH1 0x6 PUSH1 0x10 SSTORE PUSH1 0xC PUSH1 0x11 SSTORE STOP INVALID
// SourceMappings: 59:16:0:-:0;56:1;49:27;99:13;96:1;89:24;136:13;133:1;126:24;173:13;170:1;163:24;210:13;207:1;200:24;247:13;244:1;237:24;284:13;281:1;274:24;321:13;318:1;311:24;358:13;355:1;348:24;395:13;392:1;385:24;433:13;429:2;422:25;471:13;467:2;460:25;509:13;505:2;498:25;547:13;543:2;536:25;585:13;581:2;574:25;623:13;619:2;612:25;661:13;657:2;650:25;699:14;695:2;688:26;25:705
