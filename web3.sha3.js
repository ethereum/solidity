var BigNumber = require('bignumber.js');
var web3 = require('../index');
var testMethod = require('./helpers/test.method.js');

var method = 'sha3';

var tests = [{
    args: ['myString'],
    formattedArgs: ['myString'],
    result: '0x319319f831983198319881',
    formattedResult: '0x319319f831983198319881',
    call: 'web3_'+ method
}];

testMethod.runTests(null, method, tests);

