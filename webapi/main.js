'use strict';
// Stock WebApi server

var express = require('express');
var morgan  = require('morgan');
var cookieParser = require('cookie-parser');
var errorHandler = require('express-error-handler');
var methodOverride = require('method-override');
//var session = require('express-session');
var bodyParser = require('body-parser');
var swig = require('swig');
var async = require('async');
var util = require('util');
var http = require('http');
var path = require('path');
var fs = require('fs');
//var zlib = require('zlib');
var apicore = require('./apicore');

// Global Exception Handler
process.on('uncaughtException', function (err) {
	util.log('[Exception?]___________report_start');
	util.log('[Stack]\n'+err.stack);
	util.log('[Arguments] : '+err.arguments);
	util.log('[Type] : '+err.type);
	util.log('[Message] : '+err.message);
	util.log('[Exception!]___________report_end');
});


var config;
try
{
	var fname = null;
	for (var i = 2; i < process.argv.length; i++)
	{
		var arg = process.argv[i];
		if (arg.indexOf('--') != 0)
		{
			fname = arg;
			break;
		}
	}

	if (fname == null)
	{
		console.error("Config file not specified.");
		return;
	}

	fname = process.cwd() + '/' + fname;
	console.log("Using config file: %s", fname);

	//config = JSON.parse(fs.readFileSync(fname, 'utf8'));
	config = require(fname);
	if (typeof config.server == 'undefined')
	{
		console.error("Invalid config file: %s", fname);
		return;
	}
}
catch (err)
{
	console.error("Parsing failed: " + err);
	return;
}

apicore.init(config, onReadyToListen);

var app = express();

// Configuration
app.set('title', 'Stock WebApi Server');
app.set('port', config.server.port);

app.engine('.html', swig.renderFile)
app.set('view engine', 'html');
app.set('views', path.join(__dirname, '/views'));
app.set('view options', {layout: false});
app.set('trust proxy', true);

app.use(methodOverride());
//app.use(session({ resave: true, saveUninitialized: true, secret: '@_@(^_^)' }));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(cookieParser());

app.use(express.static(path.join(__dirname, 'public')));

if ('development' == app.get('env'))
{
	console.log("App: DevConfig");
	swig.setDefaults({ cache: false });
	app.set('view cache', false);
	app.use(morgan('dev'));
	app.use(errorHandler());
}

if ('production' == app.get('env'))
{
	console.log("App: ProductionConfig");
	app.use(morgan('combined'));
}




var prefix = "/a";

app.get(prefix+'/v1/hello', function(req,res) {
	var ipaddr = "RealIp:"+req.headers['x-real-ip'] +",ForwardIp:"+req.headers['x-forwarded-for']+",connIp:"+ req.connection.remoteAddress+",reqIp:"+req.ip;
	apicore.hello(res, {
		target:ipaddr
	});
});

app.post(prefix+'/v1/echo', function(req,res) {
	console.log(req.body);
	res.send(req.body);
});

app.get(prefix+'/v1/shcode/kospi200f', function(req,res) {
	res.send({'shcode':'101K9000'});
});

app.put(prefix+'/v1/kospi200f', function(req,res) {
	if (req.body.period == 'day')
	{
		apicore.KOSPI200F_feedDayItems(res, req.body.extra, req.body.items);
	}
	else if (req.body.period == 'now')
	{
		console.log("realtime data");
	}
});

app.get(prefix+'/v1/kospi200f/day', function(req,res) {
	apicore.KOSPI200F_getDays(res);
});

//////////////////////////////////////////////////////////////////////////////

function onReadyToListen(error)
{
	if (error)
	{
		console.error("Failed to init Stock WebApi server: " + error);
		return;
	}

	var server = http.createServer(app);
	server.listen(config.server.port, function() {
	      console.log("Stock WebApi Server listening on port %d in %s mode", server.address().port, app.settings.env);
	});
}

