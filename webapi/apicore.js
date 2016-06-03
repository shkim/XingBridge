'use strict';
// Stock WebApi implementation file
var mysql = require('mysql');
var async = require('async');
var util = require('util');
var path = require('path');
var fs = require('fs');
var request = require('request');
var unzip = require("unzip");


var ERRCODE_DBFAIL = "DB_ERROR";
var ERRCODE_SYSFAIL = "SYS_FAIL";
var ERRCODE_NO_UDID = "NO_UDID";
var ERRCODE_INVALID_ARGS = "INVALID_ARGS";
var ERRCODE_INVALID_PASSWD = "DIFF_PASSWD";
var ERRCODE_INVALID_SESSION = "INVALID_SESSION";
var ERRCODE_INVALID_ARC = "INVALID_FILE";
var ERRCODE_TITLE_DUPLICATE = "DUPLICATED";
var ERRCODE_TIMEOUT = "TIMEOUT";
var ERRCODE_USER_NOTFOUND = "NO_USER";
var ERRCODE_USER_INVALIDID = "INVALID_USER";

var dbConfig;
var db;

function handleDisconnect()
{
	db = mysql.createConnection(dbConfig);

	db.connect(function(err) {
		if(err) {
			console.log('Error when connecting to db:', err);
			setTimeout(handleDisconnect, 2000);
		}
	});

	db.on('error', function(err) {
		if(err.code === 'PROTOCOL_CONNECTION_LOST') {
			handleDisconnect();
		} else {
			console.log('DB error', err);
			throw err;
		}
	});

}


exports.init = function(config, initDoneCallback)
{
	// MySQL
	dbConfig = config.mysql;
	handleDisconnect();

	onCoreInit(initDoneCallback);
}

var API_VERSION = '0.1';

function onCoreInit(initDoneCallback)
{
	async.series([
		function(next) {
			db.query("SELECT now()", function(err,rows) {
				if (err)
				{
					next(err);
					return;
				}

				next(null);
			});
		},
	], function(err,results) {
		if (err)
		{
			initDoneCallback(err);
			return;
		}

		initDoneCallback(null);
	});
}

function apiError(errCode, errMsg)
{
	if (typeof errMsg != 'string')
	{
		errMsg = JSON.stringify(errMsg);
	}

	return {
		"result":errCode,
		"error":errMsg
	};
}

function apiOK(data)
{
	return {
		"result":"OK",
		"data":data
	};
}

exports.hello = function(res, params)
{
	var target = params.target;

	res.send(apiOK("Hello, " + target));
}

function insertKospi200fExtraFields(pkId, extra, item, cb)
{
	var fn = function(err,dbRes) { cb(err); }

	if (extra == 'PMA')
	{
		db.query("INSERT INTO kospi200f_day_etc (_id,end_simple,ma_10,ma_20,ma_60,ma_120) VALUES(?,?,?,?,?,?) ON DUPLICATE KEY UPDATE end_simple=VALUES(end_simple), ma_10=VALUES(ma_10), ma_20=VALUES(ma_20), ma_60=VALUES(ma_60), ma_120=VALUES(ma_120)", [pkId, item['end_simple'],item['ma_10'],item['ma_20'],item['ma_60'],item['ma_120']], fn);
	}
	else if (extra == 'MACD')
	{
		db.query("INSERT INTO kospi200f_day_etc (_id,macd_oscil,macd_1226,macd_signal9) VALUES(?,?,?,?) ON DUPLICATE KEY UPDATE macd_oscil=VALUES(macd_oscil), macd_1226=VALUES(macd_1226), macd_signal9=VALUES(macd_signal9) ", [pkId, item['macd_oscil'],item['macd_1226'],item['signal9']], fn);
	}
	else if (extra == 'RSI')
	{
		db.query("INSERT INTO kospi200f_day_etc (_id,rsi_14,rsi_signal9) VALUES(?,?,?) ON DUPLICATE KEY UPDATE rsi_14=VALUES(rsi_14), rsi_signal9=VALUES(rsi_signal9)", [pkId, item['rsi14'],item['signal9']], fn);
	}
	else
	{
		cb(new Error("Invalid extra: " + extra));
	}
}

exports.KOSPI200F_feedDayItems = function(res, extra, items)
{
	if (extra == 'PMA' || extra == 'MACD' || extra == 'RSI')
	{
	}
	else
	{
		res.send(apiError(ERRCODE_INVALID_ARGS, 'Invalid extra: '+extra));
		return;
	}

	async.eachSeries(items, function(item, cb) {
		var day8 = item['day'];
		var dayDate = day8.substring(0,4)+'-'+day8.substring(4,6)+'-'+day8.substring(6);
		db.query("SELECT * FROM kospi200f_day WHERE day=?", [dayDate], function(err,rows) {
			if (err)
			{
				console.error(err);
				res.send(apiError(ERRCODE_DBFAIL, err));
				cb(err);
				return;
			}

			var start = item['start'];
			var end = item['end'];
			var low = item['low'];
			var high = item['high'];
			var volume = Math.floor(item['quantity']);

			if (rows.length == 0)
			{
				console.log("Insert new row for " + dayDate);
				db.query("INSERT INTO kospi200f_day VALUES(NULL,?, ?,?,?,?,?)", [
					dayDate,start,high,low,end,volume], function(err,dbRes) {
					if (err)
					{
						cb(err);
						return;
					}

					insertKospi200fExtraFields(dbRes.insertId, extra, item, cb);
				}); // INSERT
			}
			else if(rows.length == 1)
			{
				if (rows[0].volume != volume
				|| rows[0].start != start
				|| rows[0].end != end
				|| rows[0].high != high
				|| rows[0].low != low)
				{
					console.error("Data mismatch @ %s (%s)", rows[0].day, dayDate);
console.error("start: %s vs %s", rows[0].start, start);
console.error("end: %s vs %s", rows[0].end, end);
console.error("high: %s vs %s", rows[0].high, high);
console.error("low: %s vs %s", rows[0].low, low);
console.error("volume: %s vs %s", rows[0].volume, volume);

					cb(new Error("Row value mismatch"));
				}
				else
				{
					insertKospi200fExtraFields(rows[0]._id, extra, item, cb);
				}
			}
			else
			{
				console.error("Invalid rows count: " + rows.length);
				cb(new Error("Invalid row count: " + rows.length));
			}
		}); // SELECT
	}, function(err) {
		if (err)
		{
			res.send(apiError('Unknown', err));
		}
		else
		{
			res.send(apiOK({'count':items.length}));
		}
	});
}


exports.KOSPI200F_getDays = function(res)
{
	db.query("SELECT date_format(day,'%Y-%m-%d') day,start,end,low,high,volume ,end_simple,ma_10,ma_20,ma_60,ma_120,macd_oscil,macd_1226,macd_signal9,rsi_14,rsi_signal9 FROM kospi200f_day A INNER JOIN kospi200f_day_etc B ON (A._id=B._id) ORDER BY day DESC LIMIT 100", [], function(err,rows) {
		if(err)
		{
			res.send(apiError(ERRCODE_DBFAIL, err));
			return;
		}

		res.send(apiOK({'title':'KOSPI200 Future', 'items':rows}));
	});
}

