// Stock WebApi Server configuration
module.exports =
{
	server: {
		port:9005,
		datadir: '/data/stock/data/'
	},

	mysql: {
		host: 'localhost',
		port: 3306,
		database: 'stock',
		user: 'shkim',
		password: 'dbdb'
	}
};

