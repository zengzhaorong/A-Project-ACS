#ifndef _USER_DB_H_
#define _USER_DB_H_

#include <sqlite3.h>
#include "public.h"

#define USERDB_FILE_NAME        "user_acs.db"
#define USERDB_TABLE		    "USER_TBL"
#define USERDB_RECOREC_TABLE	"RECOGNREC_TBL"

#define USERDB_COL_ID		    "ID"
#define USERDB_COL_NAME		    "NAME"
#define USERDB_COL_FACEPATH		"FACEPATH"
#define USERDB_COL_TIME		    "TIME"
#define USERDB_COL_CONFID		"CONFID"


/* user information */
struct db_userinfo
{
    int id;
    char name[USER_NAME_LEN];
    char facepath[DIR_PATH_LEN];    // face path
};

/* face recognize record */
struct db_recognRecord
{
    int time;       // recognize time
    int id;
    char name[USER_NAME_LEN];
    int confid;
};

int db_user_write(sqlite3 *db, struct db_userinfo *user);
int db_user_update(sqlite3 *db, struct db_userinfo *user);
int db_user_read_byId(sqlite3 *db, int id, struct db_userinfo *user);
int db_user_read_byName(sqlite3 *db, char *name, struct db_userinfo *user);
int db_user_delete_byId(sqlite3 *db, int id);
int db_user_delete_byName(sqlite3 *db, char *name);
int db_user_get_total(sqlite3 *db);
int db_user_traverse_user(sqlite3 *db, int *cursor, struct db_userinfo *user);
int db_user_check_exist(sqlite3 *db, int id);
int db_user_creat_tbl(sqlite3 *db, char *tbl_name);

int db_recorec_write(sqlite3 *db, struct db_recognRecord *recorec);
int db_recorec_delete_all(sqlite3 *db);
int db_recorec_traverse_user(sqlite3 *db, int *cursor, struct db_recognRecord *recorec);
int db_recorec_creat_tbl(sqlite3 *db, char *tbl_name);

int userdb_init(sqlite3 **ppdb);

#endif	// _USER_DB_H_