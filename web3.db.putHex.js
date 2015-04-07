var chai = require('chai');
var web3 = require('../index');
var testMethod = require('./helpers/test.method.js');

var method = 'putHex';

var tests = [{
    args: ['myDB', 'myKey', '0xb'],
    formattedArgs: ['myDB', 'myKey', '0xb'],
    result: true,
    formattedResult: true,
    call: 'db_'+ method
}];

testMethod.runTests('db', method, tests);

