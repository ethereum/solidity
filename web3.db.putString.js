var chai = require('chai');
var web3 = require('../index');
var testMethod = require('./helpers/test.method.js');

var method = 'putString';

var tests = [{
    args: ['myDB', 'myKey', 'myValue'],
    formattedArgs: ['myDB', 'myKey', 'myValue'],
    result: true,
    formattedResult: true,
    call: 'db_'+ method
}];

testMethod.runTests('db', method, tests);

