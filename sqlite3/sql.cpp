#include <cstdio>
#include <string>
#include <ctime>
#include <assert.h>
#include <sqlite3.h>

struct SqlRecord
{
	time_t tm_t;
	int temp;
	unsigned int humi;
	SqlRecord(void)
	{
		tm_t = 0;
		temp = 0;
		humi = 0;
	}
	
	void SetRecord(time_t _tm = 0, int _temp = -27300, unsigned int _humi = 0)
	{
		tm_t = _tm;
		temp = _temp;
		humi = _humi;
	}
};

int sql_tab_callback(void* pvoid, int ncolumn, char** pcolumn_text, char** pcolumn_name);
int sql_sel_callback(void* pvoid, int ncolumn, char** pcolumn_text, char** pcolumn_name);
/* create sql table */
int sql_createtab(sqlite3 *sql_fd, std::string tabname);
/* insert new data */
int sql_insert(sqlite3 *sql_fd, SqlRecord record);
/* delete data from sql */
int sql_delete(sqlite3 *sql_fd, SqlRecord record);
/* select data from sql */
int sql_select(sqlite3 *sql_fd, std::string tab_name, std::string sel_condition);

int main()
{
	int reslt = 0;
	sqlite3* sqldb = nullptr;
	int res = sqlite3_open("sht.db", &sqldb);

	if (res == SQLITE_OK)
	{
		std::string tab_name = "record";
		if(-1 != sql_createtab(sqldb, tab_name))
		{
			reslt = -1;
		}
		else
		{
			/* Insert a new row */
			time_t _tm;
			time(&_tm);
			SqlRecord var;
			var.SetRecord(_tm, 2410, 64);
			sql_insert(sqldb, var);
			
			time(&_tm);
			var.SetRecord(_tm, 3212, 66);
			sql_insert(sqldb, var);

			/* Query all of the current table */
			std::string sel_condition ="*";
			sql_select(sqldb, tab_name, sel_condition);
		}
		sqlite3_close(sqldb);
	}
	else
	{
		printf("Can't open\\create database:%s\n", sqlite3_errmsg(sqldb));
		reslt = -1;
	}
	return reslt;
}

int sql_tab_callback(void* pvoid, int ncolumn, char** pcolumn_text, char** pcolumn_name)
{
	int *tab_count = (int *)pvoid;

	if (tab_count != nullptr)
	{
		for (int i = 0; i < ncolumn; i++)
		{
			std::string str_res = pcolumn_text[i];
			sscanf(str_res.c_str(), "%d", tab_count);
		}
	}
	return 0;
}

int sql_sel_callback(void* pvoid, int ncolumn, char** pcolumn_text, char** pcolumn_name)
{
	int *sel_count = (int *)pvoid;

	if (sel_count != nullptr)
	{
		for (int i = 0; i < ncolumn; i++)
		{
			std::string str_res = pcolumn_text[i];
			if (i == (ncolumn - 1))
			{
				printf("%s\n", str_res.c_str());
			}
			else
			{
				printf("%s\t", str_res.c_str());
			}
		}
		++sel_count;
	}
	return 0;
}

/* create sql table */
int sql_createtab(sqlite3 *sql_fd, std::string tabname)
{
	assert(sql_fd != nullptr);
	
	int reslt = 0;
	char* errmsg = nullptr;
	
	/* First determine whether the table you want to operate already exists, and if it does not exist, create it */
	int tab_count = 0;
	std::string sql = "select count(*) from sqlite_master where type=\"table\" and name=\"" + tabname + "\"";
	int res = sqlite3_exec(sql_fd, sql.c_str(), sql_tab_callback, &tab_count, &errmsg);
	if (res != SQLITE_OK)
	{
		printf("%s:L%d, %s:%s\n", __FILE__, __LINE__,  __func__, errmsg);
		if (errmsg != nullptr)
		{
			sqlite3_free(errmsg);
			errmsg = nullptr;
		}
		reslt = -1;
	}
	else
	{
		if (tab_count == 0)
		{
			sql = "CREATE TABLE main." + tabname + "(tmstamp INTEGER PRIMARY KEY, temp REAL,humi REAL)";
			res = sqlite3_exec(sql_fd, sql.c_str(), nullptr, nullptr, &errmsg);
			if (res != SQLITE_OK)
			{
				printf("%s:L%d, %s:%s\n", __FILE__, __LINE__,  __func__, errmsg);
				if (errmsg != nullptr)
				{
					sqlite3_free(errmsg);
					errmsg = nullptr;
				}
				reslt = -1;
			}
		}
	}
	
	return reslt;
}

/* insert new data */
int sql_insert(sqlite3 *sql_fd, SqlRecord record)
{
	assert(sql_fd != nullptr);
	
	int reslt = 0;
	
	char* errmsg = nullptr;
	char str_tm[21] = { 0 };
	char str_temp[7] = { 0 };
	char str_humi[4] = { 0 };
	sprintf(str_tm, "%lu", uint64_t(record.tm_t));
	sprintf(str_temp, "%d", record.temp);
	sprintf(str_humi, "%u", record.humi);
	std::string sql = "INSERT INTO \"main\".\"record\" (\"tmstamp\", \"temp\", \"humi\") VALUES (" +
						std::string(str_tm) + "," + std::string(str_temp) +"," + std::string(str_humi) + ")";
	int res = sqlite3_exec(sql_fd, sql.c_str(), nullptr, nullptr, &errmsg);
	if (res != SQLITE_OK)
	{
		printf("Insert error:%s\n", errmsg);
		if (errmsg != nullptr)
		{
			sqlite3_free(errmsg);
			errmsg = nullptr;
		}
		reslt = -1;
	}
	return reslt;
}

int sql_delete(sqlite3 *sql_fd, SqlRecord record)
{
	return -1;
}

/* select data from sql */
int sql_select(sqlite3 *sql_fd, std::string tab_name, std::string sel_condition)
{
	assert(sql_fd != nullptr);
	
	int reslt = 0;
	char* errmsg = nullptr;
	int sel_count = 0;
	
	std::string sql = "SELECT " + sel_condition + " FROM main." + tab_name;
	int res = sqlite3_exec(sql_fd, sql.c_str(), sql_sel_callback, &sel_count, &errmsg);
	if (res != SQLITE_OK)
	{
		printf("Select error:%s\n", errmsg);
		if (errmsg != nullptr)
		{
			sqlite3_free(errmsg);
			errmsg = nullptr;
		}
		reslt = -1;
	}
	else
	{
		printf("a total of %d results were queried\n", sel_count);
	}
	
	return reslt;
}
