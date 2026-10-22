#! /usr/bin/env python3
import ast
import random
import re
import os
import getopt
import sys
from os import path
import tempfile
import subprocess

ENCODING = "utf-8"
SOURCE_FILE_PATTERN = r"\b\d+_error\b"
SOURCE_DIRECTORIES = [
    "libevmasm", "liblangutil", "libsolc", "libsolidity", "libsolutil", "libyul", "solc"
]
SOURCE_FILE_EXTENSIONS = [".h", ".cpp"]

# Error IDs that were previously used but have been removed from the code. They should not be reused.
# For each ID, a file and a removal commit are provided.
# Keep entries in each section sorted numerically by error ID.
REMOVED_IDS = {
    # These IDs were removed but reused before this inventory was introduced.
    # They are active again, so they are intentionally not reserved.
    # "2314", # liblangutil/ParserBase.cpp, 6965f199fd11cada51c5a97ceb72cc5e14534a17
    # "2450", # libyul/AsmAnalysis.cpp, 9820df58abfe918c5f92c6d4fa066e2da5c3bf35
    # "2657", # libyul/AsmAnalysis.cpp, ded5d721d2c97170905a536b1f996b79045c84d1
    # "3881", # libsolidity/analysis/ReferencesResolver.cpp, 66a8c7d1ab5b744736d1355ba46c9e1d5f090ac8
    # "5798", # libyul/AsmParser.cpp, e5ab68ed71460ad088788ac0bc6d94c16832bcfe
    # "5883", # libsolidity/analysis/OverrideChecker.cpp, 1d5350e32f04e991dbfc8fca402cbc8c7930e85d
    # "6546", # libsolidity/analysis/ReferencesResolver.cpp, 66a8c7d1ab5b744736d1355ba46c9e1d5f090ac8
    # "9239", # libsolidity/analysis/TypeChecker.cpp, da36400576304bc6daa924d40684b6655914e4a5

    # This ID was removed from an ErrorId documentation example, not from an actual diagnostic.
    # "3141", # liblangutil/ErrorReporter.h, c9593417207b04ca7fc6bd0b24f1009d63eaa046

    "1017", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "1054", # liblangutil/ParserBase.cpp, 0ee4a85a841f6494c16a93bd60f3a9ec1a7f2c6e
    "1093", # libsolidity/analysis/TypeChecker.cpp, 936ea6f950ffd9b4d4bfaa9cd5ae949be0df58fd
            # libsolidity/formal/SMTEncoder.cpp, 11a7763f492411d1fb77841caa48fd918dca64d4
    "1123", # libsolidity/analysis/TypeChecker.cpp, fd9050614a6089168bd4565f4b49c64d04a0ef71
    "1147", # libsolidity/formal/CHC.cpp, 694ec92688617a876d1f1d97beba6ca1b52e6906
    "1220", # libsolidity/analysis/TypeChecker.cpp, fd9050614a6089168bd4565f4b49c64d04a0ef71
    "1273", # libsolidity/analysis/TypeChecker.cpp, 3f14c904b08a358fe1b039519445439eb37d8f37
    "1305", # libyul/AsmAnalysis.cpp, f4e179ecb397dd326ffd2e576e0c8a36422fd3b5
    "1439", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "1481", # libsolidity/analysis/PostTypeContractLevelChecker.cpp, 6965f0fd48226f22df9f29dd9cf022af3264a578
    "1574", # libsolidity/analysis/ImmutableValidator.cpp, dad2bf64723cc83f0168bae0310714db01983a50
    "1665", # libsolidity/analysis/DeclarationTypeChecker.cpp, 3f14c904b08a358fe1b039519445439eb37d8f37
    "1719", # libsolidity/analysis/TypeChecker.cpp, 93c792c696b2929da2b6e2eea31a4c5f60e9188c
    "1723", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "1733", # libyul/AsmAnalysis.cpp, 0d0f2771654b14ee0e8d317a5eb22dbff8eab332
    "1741", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "1801", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "1807", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "1950", # libsolidity/formal/SMTEncoder.cpp, fa561dbd0e839ce5e198059a0eff139e5860ea89
    "1957", # liblangutil/ParserBase.cpp, 6965f199fd11cada51c5a97ceb72cc5e14534a17
            # liblangutil/ParserBase.cpp, 9adbced98e49588bcc29e181f2560c2eadaa22ac
    "2015", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "2101", # libyul/AsmAnalysis.cpp, f4e179ecb397dd326ffd2e576e0c8a36422fd3b5
    "2138", # libsolidity/experimental/analysis/TypeInference.cpp, fb99132474c177fc861305feeafd43b9cbb93b66
    "2141", # libsolidity/interface/CompilerStack.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "2177", # libsolidity/formal/SMTEncoder.cpp, e23d8f559370f5aa740782d22048a1d16fd6e4aa
    "2186", # libyul/AsmAnalysis.cpp, f4e179ecb397dd326ffd2e576e0c8a36422fd3b5
    "2202", # libevmasm/Assembly.cpp, f4e179ecb397dd326ffd2e576e0c8a36422fd3b5
    "2217", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "2332", # libsolidity/analysis/TypeChecker.cpp, e7a6534d4f34e1333bc34f17031997e495fc7f90
    "2345", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "2350", # libsolidity/formal/SMTEncoder.cpp, 5ffc27f99544d4521390c79170de985257c2cc75
    "2370", # libsolidity/analysis/TypeChecker.cpp, 2665eaa4fac3d249fa3d892487ed7b5cfd562e49
    "2399", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "2599", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "2655", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "2658", # libsolidity/analysis/ImmutableValidator.cpp, dad2bf64723cc83f0168bae0310714db01983a50
            # libsolidity/experimental/analysis/TypeInference.cpp, fb99132474c177fc861305feeafd43b9cbb93b66
    "2683", # libsolidity/formal/SMTEncoder.cpp, bd0c46abf55cd6d3beae62bd1618ad8463b4e88b
    "2703", # libevmasm/Assembly.cpp, c06a4e2f80f16d38f6d8ad23f4e045d41802eb71
    "2718", # libsolidity/analysis/ImmutableValidator.cpp, dad2bf64723cc83f0168bae0310714db01983a50
    "2837", # libsolidity/parsing/Parser.cpp, 39e3da19053fb4d892e1aba2de9ba7a5c225f68f
    "2923", # libsolidity/formal/SMTEncoder.cpp, c8cc73c80c994f84563ac474a455521d38fbcc8f
    "2934", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "3101", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "3111", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "3195", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "3263", # libsolidity/formal/SMTEncoder.cpp, fa561dbd0e839ce5e198059a0eff139e5860ea89
    "3299", # libsolidity/analysis/SyntaxChecker.cpp, ad311fae1902cf3159834de6d494b46386d0cede
    "3312", # libsolidity/analysis/TypeChecker.cpp, d41eaeba5686828b85279057f3a7da8be0f9a8f9
    "3347", # liblangutil/ParserBase.cpp, 9adbced98e49588bcc29e181f2560c2eadaa22ac
    "3408", # libsolidity/analysis/TypeChecker.cpp, e7a6534d4f34e1333bc34f17031997e495fc7f90
    "3442", # libsolidity/analysis/TypeChecker.cpp, d41eaeba5686828b85279057f3a7da8be0f9a8f9
    "3478", # libsolidity/analysis/TypeChecker.cpp, 93c792c696b2929da2b6e2eea31a4c5f60e9188c
    "3520", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "3530", # libsolidity/analysis/DeclarationTypeChecker.cpp, 0004ad876451e28de1b3aa031ab56d08d2cdb3b1
    "3570", # libsolidity/experimental/analysis/TypeRegistration.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "3573", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "3625", # libsolidity/analysis/TypeChecker.cpp, 52c49aebe80ada117f3244e73eb5e643bbbfafe1
    "3654", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "3672", # libyul/AsmAnalysis.cpp, 291c00c3decd89f3338777ce8836a974b216cd2a
    "3682", # libsolidity/formal/SMTEncoder.cpp, fa561dbd0e839ce5e198059a0eff139e5860ea89
    "3765", # libsolidity/analysis/TypeChecker.cpp, c06a4e2f80f16d38f6d8ad23f4e045d41802eb71
    "3772", # libyul/AsmParser.cpp, f04adde6641b09c8744fb1576eda87657b4d9a2d
    "3781", # libyul/AsmAnalysis.cpp, 0d0f2771654b14ee0e8d317a5eb22dbff8eab332
    "3796", # liblangutil/ParserBase.cpp, 9adbced98e49588bcc29e181f2560c2eadaa22ac
    "3876", # libsolidity/formal/SMTEncoder.cpp, fa561dbd0e839ce5e198059a0eff139e5860ea89
    "3947", # libyul/AsmAnalysis.cpp, 0d0f2771654b14ee0e8d317a5eb22dbff8eab332
    "3965", # libevmasm/Assembly.cpp, f4e179ecb397dd326ffd2e576e0c8a36422fd3b5
    "3969", # libsolidity/analysis/ImmutableValidator.cpp, dad2bf64723cc83f0168bae0310714db01983a50
    "3978", # libsolidity/analysis/TypeChecker.cpp, fda352094f0e9f586004dda48daef9788068f523
    "3997", # libsolidity/analysis/SyntaxChecker.cpp, 9adbced98e49588bcc29e181f2560c2eadaa22ac
    "4035", # libsolidity/analysis/TypeChecker.cpp, 936ea6f950ffd9b4d4bfaa9cd5ae949be0df58fd
    "4130", # libsolidity/analysis/ImmutableValidator.cpp, dad2bf64723cc83f0168bae0310714db01983a50
    "4164", # libsolidity/experimental/analysis/DebugWarner.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "4224", # libsolidity/analysis/TypeChecker.cpp, e590a99f399e0be33dfba0ab63065d7f17e1e870
    "4316", # libyul/AsmAnalysis.cpp, 67ebb206eab4863d47f67ed219ee4dbaa985819f
    "4328", # libyul/AsmAnalysis.cpp, f4e179ecb397dd326ffd2e576e0c8a36422fd3b5
    "4337", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "4496", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "4504", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "4579", # libsolidity/analysis/TypeChecker.cpp, e590a99f399e0be33dfba0ab63065d7f17e1e870
    "4599", # libsolidity/analysis/ImmutableValidator.cpp, dad2bf64723cc83f0168bae0310714db01983a50
    "4626", # libsolidity/analysis/TypeChecker.cpp, 93c792c696b2929da2b6e2eea31a4c5f60e9188c
    "4639", # libsolidity/formal/SMTEncoder.cpp, 80d743426f7c7da5e55423f6a3223145bf75b559
    "4686", # libsolidity/experimental/analysis/TypeInference.cpp, fce70ef0e34b7e6cc8d22b0fc08784758503e8d7
    "4767", # libsolidity/experimental/analysis/TypeClassRegistration.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "4794", # libsolidity/analysis/ReferencesResolver.cpp, ffdb0e37ff788afdf54ea1224a8c8184043322f9
    "4873", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "4955", # libsolidity/analysis/ReferencesResolver.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "5044", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "5073", # libsolidity/analysis/NameAndTypeResolver.cpp, efe319998110068b8bd3152230a6ff5d0a2339f1
    "5084", # libsolidity/formal/SMTEncoder.cpp, fedbea46cda1d67f00212e2d3ca51e34b8061076
    "5094", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "5096", # libsolidity/experimental/analysis/TypeRegistration.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "5104", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "5170", # libyul/AsmAnalysis.cpp, 0d0f2771654b14ee0e8d317a5eb22dbff8eab332
    "5195", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "5202", # libyul/AsmAnalysis.cpp, f4e179ecb397dd326ffd2e576e0c8a36422fd3b5
    "5256", # libsolidity/analysis/DocStringTagParser.cpp, 354f9d101530d38faaeaaf12382bf95e053e21e3
    "5262", # libsolidity/experimental/analysis/TypeRegistration.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "5348", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "5360", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "5380", # libsolidity/analysis/TypeChecker.cpp, d41eaeba5686828b85279057f3a7da8be0f9a8f9
    "5387", # libsolidity/analysis/ReferencesResolver.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "5577", # libsolidity/experimental/analysis/TypeRegistration.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "5622", # libsolidity/formal/BMC.cpp, eb56b4bfe6fe9996342b27d4fc6a4001725d4446
    "5709", # libsolidity/parsing/Parser.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "5714", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "5731", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "5755", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "5904", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "5934", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "6084", # libsolidity/formal/BMC.cpp, 07427c798c48061c608b1fded06c12bd296fa563
    "6151", # libsolidity/analysis/TypeChecker.cpp, fda352094f0e9f586004dda48daef9788068f523
    "6156", # libsolidity/formal/SMTEncoder.cpp, 78eb37d2596c9416b3fac9881414bf7e9c524de8
    "6175", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "6191", # libsolidity/formal/SMTEncoder.cpp, bd0c46abf55cd6d3beae62bd1618ad8463b4e88b
    "6387", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "6388", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "6396", # libsolidity/analysis/PostTypeContractLevelChecker.cpp, 6300a440cf1ecc2407bb456be3700a36060b2325
    "6460", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "6493", # libsolidity/analysis/DeclarationTypeChecker.cpp, 5394435bea4e553a86f872b6b2512c50bdef1628
    "6547", # libsolidity/experimental/analysis/Analysis.cpp, 194b114664c7daebc2ff68af3c573272f5d28913
    "6620", # libsolidity/experimental/analysis/TypeRegistration.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "6635", # liblangutil/ParserBase.cpp, 6965f199fd11cada51c5a97ceb72cc5e14534a17
            # liblangutil/ParserBase.cpp, 9adbced98e49588bcc29e181f2560c2eadaa22ac
    "6660", # libsolidity/formal/SMTEncoder.cpp, 4e343590635f98f7c1d0973e82f9bafc67e03466
    "6672", # libsolidity/analysis/ImmutableValidator.cpp, dad2bf64723cc83f0168bae0310714db01983a50
    "6706", # libsolidity/analysis/TypeChecker.cpp, 936ea6f950ffd9b4d4bfaa9cd5ae949be0df58fd
    "6715", # libsolidity/analysis/TypeChecker.cpp, c5685e70544c86f25d87b93048a8bffa963177c6
    "6739", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "6756", # libsolidity/formal/SMTEncoder.cpp, 78eb37d2596c9416b3fac9881414bf7e9c524de8
    "6948", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "6963", # libsolidity/analysis/TypeChecker.cpp, 93c792c696b2929da2b6e2eea31a4c5f60e9188c
    "6983", # libsolidity/analysis/TypeChecker.cpp, 93c792c696b2929da2b6e2eea31a4c5f60e9188c
    "7059", # libsolidity/parsing/Parser.cpp, 93c792c696b2929da2b6e2eea31a4c5f60e9188c
    "7079", # libyul/AsmAnalysis.cpp, d211a45aa4844821e03c38b1abcecb05dafd27fe
    "7186", # libsolidity/formal/SMTEncoder.cpp, fa561dbd0e839ce5e198059a0eff139e5860ea89
    "7223", # libyul/AsmAnalysis.cpp, f4e179ecb397dd326ffd2e576e0c8a36422fd3b5
    "7341", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "7428", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "7439", # libsolidity/parsing/Parser.cpp, 93c792c696b2929da2b6e2eea31a4c5f60e9188c
    "7484", # libsolidity/analysis/ImmutableValidator.cpp, dad2bf64723cc83f0168bae0310714db01983a50
    "7531", # libsolidity/analysis/ReferencesResolver.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "7569", # libyul/AsmAnalysis.cpp, 011f8a462d718f3034e72025b1d3d3916faa474d
    "7575", # libyul/AsmAnalysis.cpp, f4e179ecb397dd326ffd2e576e0c8a36422fd3b5
    "7637", # libsolidity/parsing/Parser.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "7645", # libsolidity/formal/SMTEncoder.cpp, 0f3924186ea02f3e335c05940dd8c16d1fb783d1
    "7653", # libsolidity/analysis/TypeChecker.cpp, fd9050614a6089168bd4565f4b49c64d04a0ef71
    "7698", # libsolidity/parsing/Parser.cpp, f73b25bb78e64a57e30ade9c7982d9c2375a3b7b
    "7733", # libsolidity/analysis/ImmutableValidator.cpp, dad2bf64723cc83f0168bae0310714db01983a50
    "7758", # libsolidity/experimental/analysis/TypeRegistration.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "7816", # libsolidity/analysis/DocStringAnalyser.cpp, cb5bfc7436ca11bdfa2f0543a3611079e3f4cbee
            # libsolidity/analysis/DocStringAnalyser.cpp, d7899a31afd8e1132ce362c522e8a47c17fd3832
    "7885", # libsolidity/formal/SMTEncoder.cpp, fa561dbd0e839ce5e198059a0eff139e5860ea89
    "7989", # libsolidity/formal/SMTEncoder.cpp, df8c6d94e30b8c66785e8c77c47a3b080270d194
    "8182", # libsolidity/formal/SMTEncoder.cpp, bd0c46abf55cd6d3beae62bd1618ad8463b4e88b
    "8185", # libsolidity/parsing/Parser.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "8195", # libsolidity/formal/SMTEncoder.cpp, 095d33714086880ded0f2ce2c7a8e4493e379613
    "8261", # libsolidity/analysis/NameAndTypeResolver.cpp, 78a097a012c8abd42b29b4dd2d38bf16a5b7cdc1
    "8273", # libsolidity/analysis/SyntaxChecker.cpp, 5d18a5d8911f6b0894242dba40a95b21cf3becd3
    "8379", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "8456", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "8532", # libsolidity/analysis/ReferencesResolver.cpp, 66a8c7d1ab5b744736d1355ba46c9e1d5f090ac8
            # libsolidity/analysis/DocStringAnalyser.cpp, cb5bfc7436ca11bdfa2f0543a3611079e3f4cbee
            # libsolidity/analysis/DocStringAnalyser.cpp, d7899a31afd8e1132ce362c522e8a47c17fd3832
    "8534", # libyul/AsmAnalysis.cpp, f4e179ecb397dd326ffd2e576e0c8a36422fd3b5
    "8809", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "8953", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "8970", # libyul/AsmAnalysis.cpp, f4e179ecb397dd326ffd2e576e0c8a36422fd3b5
    "9011", # libsolidity/formal/SMTEncoder.cpp, fa561dbd0e839ce5e198059a0eff139e5860ea89
    "9054", # libsolidity/analysis/TypeChecker.cpp, da36400576304bc6daa924d40684b6655914e4a5
    "9056", # libsolidity/formal/SMTEncoder.cpp, bd0c46abf55cd6d3beae62bd1618ad8463b4e88b
    "9085", # libsolidity/analysis/TypeChecker.cpp, f766700000e1c50f400581fc647f71daef2bb588
    "9118", # libsolidity/formal/SMTEncoder.cpp, e61b731647b88fe97ee648c4035ecb0fe210eaf0
    "9132", # libyul/AsmAnalysis.cpp, f4e179ecb397dd326ffd2e576e0c8a36422fd3b5
    "9149", # libsolidity/formal/SMTEncoder.cpp, 79f550dba94d5831be264745012cf0433556d8fb
    "9155", # libsolidity/analysis/ReferencesResolver.cpp, fc2e9ec2ff4ca397f0aa743cc44b40352894fec2
    "9159", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "9173", # libsolidity/experimental/analysis/TypeInference.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "9222", # libsolidity/parsing/DocStringParser.cpp, acd42a08c1b1fe0f69dbcd9c97ac75cc0b1b352b
    "9282", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "9390", # libsolidity/analysis/TypeChecker.cpp, fd9050614a6089168bd4565f4b49c64d04a0ef71
    "9440", # libsolidity/analysis/DocStringTagParser.cpp, 0b45168bcbf5ed5874668f21a3c3d1476cab5620
    "9547", # libyul/AsmAnalysis.cpp, 0d0f2771654b14ee0e8d317a5eb22dbff8eab332
    "9551", # libsolidity/formal/SMTEncoder.cpp, fa561dbd0e839ce5e198059a0eff139e5860ea89
    "9595", # libyul/AsmAnalysis.cpp, 011f8a462d718f3034e72025b1d3d3916faa474d
    "9599", # libsolidity/formal/SMTEncoder.cpp, bd0c46abf55cd6d3beae62bd1618ad8463b4e88b
    "9603", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "9609", # libsolidity/interface/CompilerStack.cpp, 0e8e4eacd59194f66571e136e635e1921af392e5
    "9658", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "9817", # libsolidity/experimental/analysis/TypeInference.cpp, fb99132474c177fc861305feeafd43b9cbb93b66
    "9822", # libyul/AsmAnalysis.cpp, f4e179ecb397dd326ffd2e576e0c8a36422fd3b5
    "9831", # libsolidity/experimental/analysis/TypeRegistration.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
    "9843", # libsolidity/analysis/DocStringTagParser.cpp, 8b7567f963e00ef15fd5ae97c310064658612ae9
    "9988", # libsolidity/experimental/analysis/SyntaxRestrictor.cpp, 54dc764531a3caa62e8109bfa8d5790d7be91b2a
}


def read_file(file_name):
    content = None
    _, tail = path.split(file_name)
    is_latin = tail == "invalid_utf8_sequence.sol"
    try:
        with open(file_name, "r", encoding="latin-1" if is_latin else ENCODING) as f:
            content = f.read()
    finally:
        if content is None:
            print(f"Error reading: {file_name}")
    return content


def write_file(file_name, content):
    with open(file_name, "w", encoding=ENCODING) as f:
        f.write(content)


def in_comment(source, pos):
    slash_slash_pos = source.rfind("//", 0, pos)
    lf_pos = source.rfind("\n", 0, pos)
    if slash_slash_pos > lf_pos:
        return True
    slash_star_pos = source.rfind("/*", 0, pos)
    star_slash_pos = source.rfind("*/", 0, pos)
    return slash_star_pos > star_slash_pos


def find_ids_in_source_file(file_name, id_to_file_names):
    source = read_file(file_name)
    for m in re.finditer(SOURCE_FILE_PATTERN, source):
        if in_comment(source, m.start()):
            continue
        underscore_pos = m.group(0).index("_")
        error_id = m.group(0)[0:underscore_pos]
        if error_id in id_to_file_names:
            id_to_file_names[error_id].append(file_name)
        else:
            id_to_file_names[error_id] = [file_name]


def find_ids_in_source_files(file_names):
    """Returns a dictionary with list of source files for every appearance of every id"""

    id_to_file_names = {}
    for file_name in file_names:
        find_ids_in_source_file(file_name, id_to_file_names)
    return id_to_file_names


def get_next_id(available_ids):
    assert len(available_ids) > 0, "Out of IDs"
    next_id = random.choice(list(available_ids))
    available_ids.remove(next_id)
    return next_id


def fix_ids_in_source_file(file_name, id_to_count, available_ids):
    source = read_file(file_name)

    k = 0
    destination = []
    for m in re.finditer(SOURCE_FILE_PATTERN, source):
        destination.extend(source[k:m.start()])

        underscore_pos = m.group(0).index("_")
        error_id = m.group(0)[0:underscore_pos]

        # incorrect id or id has a duplicate somewhere
        if not in_comment(source, m.start()) and (
            len(error_id) != 4 or
            error_id[0] == "0" or
            id_to_count[error_id] > 1 or
            error_id in REMOVED_IDS
        ):
            assert error_id in id_to_count
            new_id = get_next_id(available_ids)
            assert new_id not in id_to_count
            id_to_count[error_id] -= 1
        else:
            new_id = error_id

        destination.extend(new_id + "_error")
        k = m.end()

    destination.extend(source[k:])

    destination = ''.join(destination)
    if source != destination:
        write_file(file_name, destination)
        print(f"Fixed file: {file_name}")


def fix_ids_in_source_files(file_names, id_to_count, available_ids):
    """
    Fixes ids in given source files;
    id_to_count contains number of appearances of every id in sources
    """

    for file_name in file_names:
        fix_ids_in_source_file(file_name, id_to_count, available_ids)


def find_files(top_dir, sub_dirs, extensions):
    """Builds a list of files with given extensions in specified subdirectories"""

    source_file_names = []
    for directory in sub_dirs:
        for root, _, file_names in os.walk(os.path.join(top_dir, directory), onerror=lambda e: sys.exit(f"Walk error: {e}")):
            for file_name in file_names:
                _, ext = path.splitext(file_name)
                if ext in extensions:
                    source_file_names.append(path.join(root, file_name))

    return source_file_names


def find_source_files(search_dir):
    return find_files(
        search_dir,
        SOURCE_DIRECTORIES,
        SOURCE_FILE_EXTENSIONS
    )


def find_ids_in_test_file(file_name):
    source = read_file(file_name)
    pattern = r"^// (.*Error|Warning|Info) \d\d\d\d:"
    return {m.group(0)[-5:-1] for m in re.finditer(pattern, source, flags=re.MULTILINE)}


def find_ids_in_test_files(file_names):
    """Returns a set containing all ids in tests"""

    ids = set()
    for file_name in file_names:
        ids |= find_ids_in_test_file(file_name)
    return ids


def find_ids_in_cmdline_test_err(file_name):
    source = read_file(file_name)
    pattern = r' \(\d\d\d\d\):'
    return {m.group(0)[-6:-2] for m in re.finditer(pattern, source, flags=re.MULTILINE)}


def print_ids(ids):
    for k, error_id in enumerate(sorted(ids)):
        if k % 10 > 0:
            print(" ", end="")
        elif k > 0:
            print()
        print(error_id, end="")


def print_ids_per_file(ids, id_to_file_names, top_dir):
    file_name_to_ids = {}
    for error_id in ids:
        for file_name in id_to_file_names[error_id]:
            relpath = path.relpath(file_name, top_dir)
            if relpath not in file_name_to_ids:
                file_name_to_ids[relpath] = []
            file_name_to_ids[relpath].append(error_id)

    for file_name in sorted(file_name_to_ids):
        print(file_name)
        for error_id in sorted(file_name_to_ids[file_name]):
            print(f" {error_id}", end="")
        print()


def examine_id_coverage(top_dir, source_id_to_file_names, new_ids_only=False):
    test_sub_dirs = [
        path.join("test", "libsolidity", "natspecJSON"),
        path.join("test", "libsolidity", "smtCheckerTests"),
        path.join("test", "libsolidity", "syntaxTests"),
        path.join("test", "libyul", "yulSyntaxTests")
    ]
    test_file_names = find_files(
        top_dir,
        test_sub_dirs,
        [".sol", ".yul"]
    )
    source_ids = source_id_to_file_names.keys()
    test_ids = find_ids_in_test_files(test_file_names)

    # special case, we are interested in warnings which are ignored by regular tests:
    # Warning (1878): SPDX license identifier not provided in source file. ....
    # Warning (3420): Source file does not specify required compiler version!
    test_ids |= find_ids_in_cmdline_test_err(path.join(top_dir, "test", "cmdlineTests", "error_codes", "err"))

    # white list of ids which are not covered by tests
    white_ids = {
        "9804", # Tested in test/libyul/ObjectParser.cpp.
        "1544",
        "1749",
        "2674",
        "6367",
        "8387",
        "3805", # "This is a pre-release compiler version, please do not use it in production."
                # The warning may or may not exist in a compiler build.
        "4591", # "There are more than 256 warnings. Ignoring the rest."
                # Due to 3805, the warning lists look different for different compiler builds.
        "1920", # Unimplemented feature error from YulStack (currently there are no UnimplementedFeatureErrors thrown by libyul)
        "7053", # Unimplemented feature error (parsing stage), currently has no tests
        "2339", # SMTChecker, covered by CL tests
        "6240", # SMTChecker, covered by CL tests
        "2788", # SMTChecker: BMC: verification condition(s) could not be proved
        "1733", # AsmAnalysis: expecting bool expression (everything is implicitly bool without types in Yul)
        "9547", # AsmAnalysis: assigning incompatible types in Yul (whitelisted as there are currently no types)
        "5026", # ContractLevelChecker: too difficult to exceed transient storage max size due to only value types supported.
        "1049", # AsmAnalysis: SLOTNUM only available for Amsterdam-compatible VMs. Only reachable once Amsterdam
                # becomes the default EVM version.
    }
    assert len(test_ids & white_ids) == 0, "The sets are not supposed to intersect"
    test_ids |= white_ids

    test_only_ids = test_ids - source_ids
    source_only_ids = source_ids - test_ids

    if not new_ids_only:
        print(f"IDs in source files: {len(source_ids)}")
        print(f"IDs in test files  : {len(test_ids)} ({len(test_ids) - len(source_ids)})")
        print()

        if len(test_only_ids) != 0:
            print("Error. The following error codes found in tests, but not in sources:")
            print_ids(test_only_ids)
            return False

        if len(source_only_ids) != 0:
            print("The following error codes found in sources, but not in tests:")
            print_ids_per_file(source_only_ids, source_id_to_file_names, top_dir)
            print("\n\nPlease make sure to add appropriate tests.")
            return False

    old_source_only_ids = {
        "1218",
        "1584",
        "1823",
        "1988",
        "2066",
        "2833",
        "3356",
        "3893",
        "3996",
        "4010",
        "4458",
        "4802",
        "4902",
        "5272",
        "5798",
        "5840",
        "7128",
        "7400",
        "7589",
        "7593",
        "7649",
        "7710",
        "8065",
        "8084",
        "8140",
        "8158",
        "8312",
        "8592",
        "9134",
        "9609",
    }

    new_source_only_ids = source_only_ids - old_source_only_ids
    if len(new_source_only_ids) != 0:
        print("The following new error code(s), not covered by tests, found:")
        print_ids(new_source_only_ids)
        print(
            "\nYou can:\n"
            "- create appropriate test(s);\n"
            "- add the error code(s) to old_source_only_ids in error_codes.py\n"
            "  (to silence the checking script, with a promise to add a test later);\n"
            "- add the error code(s) to white_ids in error_codes.py\n"
            "  (for rare cases when the error is not supposed to be tested)"
        )
        return False

    return True

def find_ids_in_branch(branch_commit):
    """Returns a set of error codes present in the given branch/commit"""

    with tempfile.TemporaryDirectory() as tmp_dir:
        # Create a temporary worktree to examine the target branch
        subprocess.run(
            ['git', 'worktree', 'add', tmp_dir, branch_commit],
            check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
        )

        try:
            source_file_names = find_source_files(tmp_dir)
            ids = set(find_ids_in_source_files(source_file_names).keys())
            assert ids, "Error IDs weren't found"
            return ids
        finally:
            subprocess.run(
                ['git', 'worktree', 'remove', '--force', tmp_dir],
                check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
            )


def find_newly_removed_ids(base_ids, current_ids):
    """Returns IDs removed by the current change that are not inventoried."""

    return set(base_ids) - set(current_ids) - REMOVED_IDS


def find_deleted_inventory_ids(base_removed_ids, current_removed_ids):
    """Returns IDs deleted from the removed-ID inventory by the current change."""

    return set(base_removed_ids) - set(current_removed_ids)


def find_inventory_ids_in_branch(branch_commit):
    """Returns the removed-ID inventory stored in the given commit."""

    source = subprocess.check_output(
        ['git', 'show', f'{branch_commit}:scripts/error_codes.py'],
        universal_newlines=True
    )
    module = ast.parse(source)
    for statement in module.body:
        if not isinstance(statement, ast.Assign):
            continue
        if any(isinstance(target, ast.Name) and target.id == 'REMOVED_IDS' for target in statement.targets):
            inventory = ast.literal_eval(statement.value)
            assert isinstance(inventory, set), "REMOVED_IDS must be a set"
            return inventory
    return set()


def check_removed_error_codes_between_branches(current_ids, target_ref):
    """
    Checks that removed codes are inventoried and inventory entries are not deleted.
    Returns True if the current branch preserves the removed-ID inventory.
    """

    # Get the merge base (common ancestor) with the configured target revision.
    try:
        merge_base = subprocess.check_output(
            ['git', 'merge-base', target_ref, 'HEAD'],
            universal_newlines=True
        ).strip()
    except subprocess.CalledProcessError:
        print(f"Error: Could not find merge base with {target_ref}")
        return False

    # Get error codes from the merge base
    try:
        target_ids = find_ids_in_branch(merge_base)
    except subprocess.CalledProcessError as e:
        print(f"Error accessing target branch: {e}")
        return False

    try:
        target_removed_ids = find_inventory_ids_in_branch(merge_base)
    except (subprocess.CalledProcessError, SyntaxError, ValueError) as e:
        print(f"Error reading removed-ID inventory from {merge_base}: {e}")
        return False

    current_ids = set(current_ids)
    removed_ids = find_newly_removed_ids(target_ids, current_ids)
    deleted_inventory_ids = find_deleted_inventory_ids(target_removed_ids, REMOVED_IDS)
    ok = True

    if len(removed_ids) > 0:
        print(f"Error: The following error codes were removed since {merge_base} but not added to REMOVED_IDS:")
        print_ids(removed_ids)
        print("\nPlease add these codes to the REMOVED_IDS set in scripts/error_codes.py")
        ok = False

    if len(deleted_inventory_ids) > 0:
        print("Error: The following error codes were deleted from REMOVED_IDS:")
        print_ids(deleted_inventory_ids)
        print("\nRemoved error codes must remain permanently reserved")
        ok = False

    if ok:
        print(f"No removed-ID inventory violations found since {merge_base}")

    return ok


def main(argv):
    check = False
    fix = False
    no_confirm = False
    examine_coverage = False
    next_id = False
    check_removed = False
    check_removed_target = None
    try:
        opts, _args = getopt.getopt(
            argv,
            "",
            ["check", "fix", "no-confirm", "examine-coverage", "next", "check-removed="],
        )
    except getopt.GetoptError as error:
        sys.exit(f"Error: {error}")

    for opt, _arg in opts:
        if opt == "--check":
            check = True
        elif opt == "--fix":
            fix = True
        elif opt == "--no-confirm":
            no_confirm = True
        elif opt == "--examine-coverage":
            examine_coverage = True
        elif opt == "--next":
            next_id = True
        elif opt == "--check-removed":
            check_removed = True
            check_removed_target = _arg

    if [check, fix, examine_coverage, next_id, check_removed].count(True) != 1:
        print(
            "usage: python error_codes.py --check | --fix [--no-confirm] | "
            "--examine-coverage | --next | --check-removed TARGET"
        )
        sys.exit(1)

    cwd = os.getcwd()

    source_file_names = find_source_files(cwd)
    source_id_to_file_names = find_ids_in_source_files(source_file_names)

    if check_removed:
        res = 0 if check_removed_error_codes_between_branches(source_id_to_file_names, check_removed_target) else 1
        sys.exit(res)

    ok = True
    for error_id in sorted(source_id_to_file_names):
        if len(error_id) != 4:
            print(f"ID {error_id} length != 4")
            ok = False
        if error_id[0] == "0":
            print(f"ID {error_id} starts with zero")
            ok = False
        if error_id in REMOVED_IDS:
            print(f"ID {error_id} was previously removed and cannot be reused")
            ok = False
        if len(source_id_to_file_names[error_id]) > 1:
            print(f"ID {error_id} appears {len(source_id_to_file_names[error_id])} times")
            ok = False

    if examine_coverage:
        if not ok:
            print("Incorrect IDs have to be fixed before applying --examine-coverage")
            sys.exit(1)
        res = 0 if examine_id_coverage(cwd, source_id_to_file_names) else 1
        sys.exit(res)

    ok &= examine_id_coverage(cwd, source_id_to_file_names, new_ids_only=True)

    random.seed()

    available_ids = {str(error_id) for error_id in range(1000, 10000)} - source_id_to_file_names.keys() - REMOVED_IDS

    if next_id:
        if not ok:
            print("Incorrect IDs have to be fixed before applying --next")
            sys.exit(1)
        next_id = get_next_id(available_ids)
        print(f"Next ID: {next_id}")
        sys.exit(0)

    if ok:
        print("No incorrect IDs found")
        sys.exit(0)

    if check:
        sys.exit(1)

    assert fix, "Unexpected state, should not come here without --fix"

    if not no_confirm:
        answer = input(
            "\nDo you want to fix incorrect IDs?\n"
            "Please commit current changes first, and review the results when the script finishes.\n"
            "[Y/N]? "
        )
        while len(answer) == 0 or answer not in "YNyn":
            answer = input("[Y/N]? ")
        if answer not in "yY":
            sys.exit(1)

    # number of appearances for every id
    source_id_to_count = { error_id: len(file_names) for error_id, file_names in source_id_to_file_names.items() }

    fix_ids_in_source_files(source_file_names, source_id_to_count, available_ids)
    print("Fixing completed")
    sys.exit(2)


if __name__ == "__main__":
    main(sys.argv[1:])
