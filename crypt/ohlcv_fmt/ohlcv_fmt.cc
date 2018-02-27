#include <iostream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <string_util.h>
#include <util.h>
#include "ohlcv_fmt.h"


using namespace std;


string feeds_dir = "/home/data/crypt_data/";
string write_dir = "/home/data/crypt_ohlcv/";

CryptoFeed_T current_feed = Feed_Unknown;


int config();
int processFeeds();
CryptoFeed_T getFeed( string path );


void test();

	
int main()
{
	int rc;

	//test();

	rc = config();
	if (rc)
	{
		return rc;
	}
	cout<< "Config completed."<< endl<< endl;

	processFeeds();

    return 0;
}


int config()
{
	string var, value;
	char line[1024];
	DIR *dr;
	size_t i;

	ifstream cfgs( CRYPT_CONFIG_FILE );
	if (cfgs.fail())
	{
		cerr << "Failed to open config file '"<< CRYPT_CONFIG_FILE
			 << "'. Make sure the config file exists with read permissions." << endl;
		return -1;
	}

	while (cfgs.good())
	{

		cfgs.getline(line, 1023);
		string lnstr(line);
		i = lnstr.find("=");
		if (i == string::npos)
		{
			continue;
		}
		var = lnstr.substr(0, i);
		value = lnstr.substr(i + 1);
		value = common::trim(value);
		if (var.find("feeds_dir") != string::npos)
		{
			feeds_dir = common::trim(value);
			if (feeds_dir[feeds_dir.size() - 1] != '/')
			{
				feeds_dir.push_back('/');
			}
			//cout << feeds_dir.c_str() << endl;
		}
		else if (var.find("write_dir") != string::npos)
		{
			// make sure write directory extsts with write permissions
			write_dir = common::trim(value);
			if (write_dir[write_dir.size() - 1] != '/')
			{
				write_dir.push_back('/');
			}
			string test_dir, test_file;
			test_dir = write_dir + "test_dir/";
			test_file = test_dir + "test_file.txt";

			if (  ( ( dr = opendir(write_dir.c_str())) == NULL ) )
			{
				cout<< "Test directory does not exist. Creating... ";
				if ( mkdir(write_dir.c_str(), 0775) != 0 )
				{
					cerr << endl<< "Failed to open or create the write directory: "<< write_dir.c_str()<< endl;
					return -2;
				}
				else if ( mkdir(test_dir.c_str(), 0775) != 0 )
				{
					cerr << endl<< "Failed to open or create the write test directory: "<< test_dir.c_str()<< endl;
					return -3;
				}
				cout<< "done."<< endl;
			}
			else
			{
				closedir(dr);
				if (  ( dr = opendir(test_dir.c_str() ) ) == NULL )
				{
					cout<< "Test directory does not exist. Creating... ";
					if ( mkdir(test_dir.c_str(), 0775) != 0 )
					{
						cerr << endl<< "Failed to open or create the write test directory: "<< test_dir.c_str()<< endl;
						return -3;
					}
					cout<< "done."<< endl;
				}
				else
				{
					closedir( dr );
				}
			}

			ofstream wfs( test_file );
			if ( wfs.fail() )
			{
				cerr << "Failed to open or create the write test file: "<< test_file.c_str()<< endl;
				return -3;
			}
			string cmd( "rm -rf " );
			cmd = cmd + test_dir.c_str();
			system( cmd.c_str() );
		}
	}

	return 0;
}

/** @brief Converts each feed file and writs it to the destination csv file in the right format */
int convertFeed( string readFile, string writeFile )
{
	ifstream ifs( readFile.c_str() );
	if (ifs.fail())
	{
		cerr << "Failed to open feed csv: " << readFile.c_str()<< endl;
		return -1;
	}
	// We are going to convert i-min bars to daily bars.
	writeFile = common::strrep1( writeFile, "1-min", "daily" );

	cout<< " Processing feed csv file: "<< readFile.c_str()<< endl;
	cout<< "   Writing to csv file: "<< writeFile.c_str()<< endl;

	return 0;
}


int processFeeds()
{
	vector<string> feeds;
	int rc;

	rc = common::listdir( feeds, feeds_dir );
	if ( rc )
	{
		return -1;	// could not find or could not read the feeds directory
	}

	string cmd = string("rm -rf ") + write_dir + "/*";
	system( cmd.c_str() );

	for ( size_t i = 0; i < feeds.size(); ++i )
	{
		if ( (feeds[i] != string("bak"))  &&  	// directories to skip
			 (feeds[i] != string("zips")) )		// directories to ignore
		{
			vector<string> fxfiles;
			string fdir = feeds_dir + feeds[i] + "/";
			string wdir = write_dir + feeds[i] + "/";

			common::create_empty_dir( wdir );
			current_feed = getFeed( fdir );

			rc = common::listdir( fxfiles, fdir );
			if ( rc == 0 )
			{
				for ( size_t j = 0; j < fxfiles.size(); ++j )
				{
					convertFeed( fdir + fxfiles[j], wdir + fxfiles[j] );
				}
			}
		}
	}

	return 0;
}

CryptoFeed_T getFeed( string path )
{
	CryptoFeed_T fd = Feed_Unknown;

	if ( common::contains( path, "bittrex") )
	{
		fd = Feed_BittrexHistory;
	}
	else if ( common::contains( path, "bitcoin") )
	{
		fd = Feed_BitcoinHistory;
	}
	return fd;
}


/*-------------------------------------------------------------------------
 * Test functions.
 */

void dir_test()
{
	vector<string> dirEntries;
	int rc;

	rc = common::listdir( dirEntries, feeds_dir );
	if ( rc ) return;
	for ( size_t i = 0; i < dirEntries.size(); ++i )
	{
		cout<< " ["<< dirEntries[i].c_str() << "]" << endl;
	}
	cout<< endl;

}


void test()
{
	dir_test();
}

